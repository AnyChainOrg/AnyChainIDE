#include "XWCBackStage.h"

#include <QProcess>
#include <QDebug>
#include <QRegExp>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTime>

#include "DataDefine.h"
#include "IDEUtil.h"

#include "datarequire/DataRequireManager.h"

static const int NODE_RPC_PORT = 60320;//node端口  test    formal = test+10
static const int CLIENT_RPC_PORT = 60321;//client端口  test    formal = test+10

class XWCBackStage::DataPrivate
{
public:
    DataPrivate(int type)
        :chaintype(type)
        ,nodeProc(new QProcess())
        ,startNode(false)
        ,clientProc(new QProcess())
        ,startClient(false)
    {
        nodePort = NODE_RPC_PORT + 10*(type-1);
        clientPort = CLIENT_RPC_PORT + 10*(type-1);

        dataPath = 1 == type ? "/testxwc/testnet" : "/formalxwc";

        dataRequire = new DataRequireManager("127.0.0.1",QString::number(clientPort));
    }
    ~DataPrivate()
    {
        if(dataRequire)
        {
            delete dataRequire;
            dataRequire = nullptr;
        }
        if(clientProc )
        {
            if(clientProc->state() == QProcess::Running)
            {
                clientProc->close();
            }
            delete clientProc;
            clientProc = nullptr;

        }
        if(nodeProc)
        {
            if(nodeProc->state() == QProcess::Running)
            {
                nodeProc->close();
            }
            delete nodeProc;
            nodeProc = nullptr;
        }
    }
public:
    int chaintype;
    int nodePort;
    int clientPort;
    QString dataPath;
    QProcess* nodeProc;
    bool startNode;
    QProcess* clientProc;
    bool startClient;
    DataRequireManager *dataRequire;
};


XWCBackStage::XWCBackStage(int type,QObject *parent)
        : BackStageBase(parent)
        ,_p(new DataPrivate(type))
{

}

XWCBackStage::~XWCBackStage()
{
    qDebug()<<"delete "<<_p->chaintype<<" xwcstage";
    delete _p;
    _p = nullptr;
}

void XWCBackStage::startExe(const QString &appDataPath)
{
    //设置数据存储路径
    QString str = appDataPath;
    str.replace("\\","/");
    _p->dataPath =str + _p->dataPath;
    //先启动node，然后启动client
    startNodeProc();
}

bool XWCBackStage::exeRunning()
{
    return _p->clientProc->state() == QProcess::Running && _p->nodeProc->state() == QProcess::Running && _p->dataRequire->isConnected();
}

QProcess *XWCBackStage::getProcess() const
{
    return _p->clientProc;
}

void XWCBackStage::ReadyClose()
{
    disconnect(_p->clientProc,&QProcess::stateChanged,this,&XWCBackStage::onClientExeStateChanged);
    disconnect(_p->clientProc,&QProcess::readyReadStandardError,this,&XWCBackStage::readClientStandError);
    disconnect(_p->clientProc,&QProcess::readyReadStandardOutput,this,&XWCBackStage::readClientStandOutput);
    disconnect(_p->nodeProc,&QProcess::stateChanged,this,&XWCBackStage::onNodeExeStateChanged);
    disconnect(_p->nodeProc,&QProcess::readyReadStandardError,this,&XWCBackStage::readNodeStandError);
    disconnect(_p->nodeProc,&QProcess::readyReadStandardOutput,this,&XWCBackStage::readNodeStandOutput);
    if(exeRunning())
    {
        QSharedPointer<QEventLoop> loop = QSharedPointer<QEventLoop>(new QEventLoop());

        connect(_p->dataRequire,&DataRequireManager::requireResponse,[&loop,this](const QString &_rpcId,const QString &message){
            if(_rpcId == "id-lock-onCloseIDE")
            {
                rpcPostedSlot( "id-witness_node_stop",IDEUtil::toJsonFormat( "witness_node_stop", QJsonArray()));
            }
            else if(_rpcId == "id-witness_node_stop")
            {
                if(loop && loop->isRunning())
                {
                    //先关node，然后关client
                    _p->nodeProc->waitForFinished(-1);
                    _p->clientProc->waitForFinished(-1);
                    qDebug()<<"close xwcstage "<<_p->chaintype<<" finish";
                    loop->quit();
                }
            }
        });
        rpcPostedSlot("id-lock-onCloseIDE",IDEUtil::toJsonFormat( "lock", QJsonArray()));

        loop->exec();
    }
    else
    {
        if(_p->clientProc)
        {
            if(_p->clientProc->state() == QProcess::Running)
            {
                _p->clientProc->close();
            }
            delete _p->clientProc;
            _p->clientProc = nullptr;
        }
        if(_p->nodeProc)
        {
            if(_p->nodeProc->state() == QProcess::Running)
            {
                _p->nodeProc->close();
            }
            delete _p->nodeProc;
            _p->nodeProc = nullptr;
        }
    }
    emit exeClosed();
}

void XWCBackStage::startNodeProc()
{
    connect(_p->nodeProc,&QProcess::stateChanged,this,&XWCBackStage::onNodeExeStateChanged);
    connect(_p->nodeProc,&QProcess::readyReadStandardError,this,&XWCBackStage::readNodeStandError);
    connect(_p->nodeProc,&QProcess::readyReadStandardOutput,this,&XWCBackStage::readNodeStandOutput);
    QStringList strList;
    strList << "--data-dir=" +_p->dataPath
            << QString("--rpc-endpoint=127.0.0.1:%1").arg(_p->nodePort);

    _p->nodeProc->setWorkingDirectory(QCoreApplication::applicationDirPath()+QDir::separator()+DataDefine::BACKSTAGE_DIR);
    if(1 == _p->chaintype)
    {//测试链
        //启动前，先判断是否需要复制config到对应目录
        QFile conf(_p->dataPath+"/config.ini");
        if(conf.exists() && conf.open(QIODevice::ReadOnly|QIODevice::Text)){
            if(!QString(conf.readAll()).contains("1.6.25")){
                conf.close();
                conf.remove();
                QFile::copy(QCoreApplication::applicationDirPath()+"/"+DataDefine::XWC_TEST_CONFIG_PATH,_p->dataPath+"/config.ini");
            }
            else{
                conf.close();
            }
        }
        else{
            QDir dir(_p->dataPath);
            if(!dir.exists()){
                dir.mkpath(_p->dataPath);
            }

            QFile::copy(QCoreApplication::applicationDirPath()+"/"+DataDefine::XWC_TEST_CONFIG_PATH,_p->dataPath+"/config.ini");
        }
        qDebug() << "start xwc_node_test " << strList;
        _p->nodeProc->start(QCoreApplication::applicationDirPath()+QDir::separator()+DataDefine::XWC_NODE_TEST_EXE,strList);
    }
    else if(2 == _p->chaintype)
    {//正式链
        qDebug() << "start xwc_node_formal " << strList;
        _p->nodeProc->start(QCoreApplication::applicationDirPath()+QDir::separator()+DataDefine::XWC_NODE_FORMAL_EXE,strList);
    }

}

void XWCBackStage::startClientProc()
{
    connect(_p->clientProc,&QProcess::stateChanged,this,&XWCBackStage::onClientExeStateChanged);
    connect(_p->clientProc,&QProcess::readyReadStandardError,this,&XWCBackStage::readClientStandError);
    connect(_p->clientProc,&QProcess::readyReadStandardOutput,this,&XWCBackStage::readClientStandOutput);

    QStringList strList;
    strList << "--wallet-file=" + _p->dataPath + "/wallet.json"
            << QString("--server-rpc-endpoint=ws://127.0.0.1:%1").arg(_p->nodePort)
            << QString("--rpc-endpoint=127.0.0.1:%1").arg(_p->clientPort);
    _p->clientProc->setWorkingDirectory(_p->dataPath);
    _p->clientProc->setWorkingDirectory(QCoreApplication::applicationDirPath()+QDir::separator()+DataDefine::BACKSTAGE_DIR);
    if(1 == _p->chaintype)
    {
        qDebug()<<"start hx_client_test"<<strList;
        _p->clientProc->start(QCoreApplication::applicationDirPath()+QDir::separator()+DataDefine::XWC_CLIENT_TEST_EXE,strList);
    }
    else if(2 == _p->chaintype)
    {
        qDebug()<<"start hx_client_formal"<<strList;
        _p->clientProc->start(QCoreApplication::applicationDirPath()+QDir::separator()+DataDefine::XWC_CLIENT_FORMAL_EXE,strList);
    }
}

void XWCBackStage::readNodeStandError()
{
    QString str = _p->nodeProc->readAllStandardError();
    if(str.isEmpty()) return;
//    qDebug() << "node exe standardError: " << str ;
    int chainFlag = static_cast<int>(_p->chaintype==1?DataDefine::NODE_ERROR_TEST_TYPE:DataDefine::NODE_ERROR_FORMAL_TYPE);
    emit AdditionalOutputMessage(str,chainFlag);
    checkNodeMessage(str);
}

void XWCBackStage::readNodeStandOutput()
{
    QString str = _p->nodeProc->readAllStandardOutput();
    if(str.isEmpty()) return;
//    qDebug() << "node exe standardOut: " << str ;
    int chainFlag = static_cast<int>(_p->chaintype==1?DataDefine::NODE_OUT_TEST_TYPE:DataDefine::NODE_OUT_FORMAL_TYPE);
    emit AdditionalOutputMessage(str,chainFlag);
}

void XWCBackStage::readClientStandError()
{
    QString str = _p->clientProc->readAllStandardError();
    if(str.isEmpty()) return;
//    qDebug() << "client exe standardError: " << str ;
    int chainFlag = static_cast<int>(_p->chaintype==1?DataDefine::CLIENT_ERROR_TEST_TYPE:DataDefine::CLIENT_ERROR_FORMAL_TYPE);
    emit AdditionalOutputMessage(str,chainFlag);
//    checkClientMessage(str);

}

void XWCBackStage::readClientStandOutput()
{
    QString str = _p->clientProc->readAllStandardOutput();
    if(str.isEmpty()) return;
//    qDebug() << "client exe standardOut: " << str ;
    int chainFlag = static_cast<int>(_p->chaintype==1?DataDefine::CLIENT_OUT_TEST_TYPE:DataDefine::CLIENT_OUT_FORMAL_TYPE);
    emit AdditionalOutputMessage(str,chainFlag);
    checkClientMessage(str);
}

void XWCBackStage::checkNodeMessage(const QString &message)
{
    if(_p->startNode) return;
    if(message.contains("Chain ID") || message.contains("Chain ") || message.contains("ID is") || message.contains("D is"))
    {
        qDebug()<<"find chain id is:"<<message;
        _p->startNode = true;
        startClientProc();
    }
}

void XWCBackStage::checkClientMessage(const QString &message)
{
    if(_p->startClient) return;
    if(message.contains("locked") || message.contains("new"))
    {
        qDebug()<<"find listening for incoming:"<<message;
        _p->startClient = true;
        initSocketManager();
    }
}

void XWCBackStage::onNodeExeStateChanged()
{
    if(_p->nodeProc->state() == QProcess::Starting)
    {
    }
    else if(_p->nodeProc->state() == QProcess::Running)
    {
        qDebug() << QString("xwc_node(_test) %1 is running").arg(_p->chaintype);
    }
    else if(_p->nodeProc->state() == QProcess::NotRunning)
    {
        qDebug()<<QString("xwc_node(_test) %1 is notrunning :%2").arg(_p->chaintype).arg(_p->nodeProc->errorString());
        emit exeNotRunning();
    }
}

void XWCBackStage::onClientExeStateChanged()
{
    if(_p->clientProc->state() == QProcess::Starting)
    {
    }
    else if(_p->clientProc->state() == QProcess::Running)
    {
        qDebug() << QString("xwc_client(_test) %1 is running").arg(_p->chaintype);

    }
    else if(_p->clientProc->state() == QProcess::NotRunning)
    {
        qDebug() << QString("xwc_client(_test) %1 not running :%2").arg(_p->chaintype).arg(_p->clientProc->errorString());
        emit exeNotRunning();
    }
}

void XWCBackStage::initSocketManager()
{
    connect(_p->dataRequire,&DataRequireManager::requireResponse,this,&XWCBackStage::rpcReceivedSlot);
    connect(_p->dataRequire,&DataRequireManager::requireOvertime,this,&BackStageBase::rpcOvertime);
    connect(_p->dataRequire,&DataRequireManager::connectFinish,this,&BackStageBase::exeStarted);

    qDebug()<<"start dateRequire";
    _p->dataRequire->startManager(DataRequireManager::WEBSOCKET);
}

void XWCBackStage::rpcPostedSlot(const QString & id, const QString & cmd)
{
    _p->dataRequire->requirePosted(id,cmd);
}

void XWCBackStage::rpcReceivedSlot(const QString &id, const QString &message)
{
//    QString result = message.mid( QString("{\"id\":32800,\"jsonrpc\":\"2.0\",").size());
//    result = result.left( result.size() - 1);
//    result.prepend("{");
//    result.append("}");
    emit rpcReceived(id,message);
}
