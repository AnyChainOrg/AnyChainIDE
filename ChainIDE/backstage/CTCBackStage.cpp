#include "CTCBackStage.h"

#include <QTimer>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QProcess>

#include "DataDefine.h"
#include "IDEUtil.h"
//#include "popwidget/commondialog.h"

#include "datarequire/DataRequireManager.h"

static const int NODE_RPC_PORT = 60320;//node端口  test    formal = test+10
static const int CLIENT_RPC_PORT = 60321;//client端口  test    formal = test+10

static const QString RPC_USER = "a";
static const QString RPC_PASSWORD = "b";

class CTCBackStage::DataPrivate
{
public:
    DataPrivate(int type)
        :chaintype(type)
        ,nodeProc(new QProcess)
    {
        nodePort = NODE_RPC_PORT + 10*(type-1);
        clientPort = CLIENT_RPC_PORT + 10*(type-1);

        dataPath = 1 == type ? "/testctc" : "/formalctc";

        dataRequire = new DataRequireManager("127.0.0.1",QString::number(clientPort));
    }
    ~DataPrivate()
    {
        if(nodeProc)
        {
            if(nodeProc->state() == QProcess::Running)
            {
                nodeProc->close();
            }
            delete nodeProc;
            nodeProc = nullptr;
        }
        if(dataRequire)
        {
            delete dataRequire;
            dataRequire = nullptr;
        }
    }
public:
    int chaintype;
    int nodePort;
    int clientPort;
    QString dataPath;
    QProcess* nodeProc;
    QTimer    timerForStartExe;
    QString currentCMD;
    DataRequireManager *dataRequire;
};

CTCBackStage::CTCBackStage(int type,QObject *parent)
    : BackStageBase(parent)
    ,_p(new DataPrivate(type))
{

}

CTCBackStage::~CTCBackStage()
{
//    qDebug()<<"delete ctcstage "<<_p->chaintype;
    delete _p;
    _p = nullptr;
}

void CTCBackStage::startExe(const QString &appDataPath)
{
    //设置数据存储路径
    QString str = appDataPath;
    str.replace("\\","/");
    _p->dataPath =str + _p->dataPath;

    connect(_p->nodeProc,&QProcess::stateChanged,this,&CTCBackStage::onNodeExeStateChanged);
    connect(_p->nodeProc,&QProcess::readyReadStandardError,this,&CTCBackStage::readNodeStandError);
    connect(_p->nodeProc,&QProcess::readyReadStandardOutput,this,&CTCBackStage::readNodeStandOutput);

    //先确保目录存在
    QString dataPath = _p->dataPath;
    QDir dir(dataPath);
    if(!dir.exists())
    {
        dir.mkpath(dir.path());
    }

    QStringList strList;
    strList << "--data-dir" << dataPath
            << "--rpcuser"<<RPC_USER<<"--rpcpassword"<<RPC_PASSWORD<<"--server"
            <<"--rpcendpoint"<<QString("127.0.0.1:%1").arg(_p->nodePort)
            <<"--httpport"<<QString::number(_p->clientPort);

    if(1 == _p->chaintype)
    {//测试链
        qDebug() << "start ctc_test.exe " << strList;
        _p->nodeProc->start(QCoreApplication::applicationDirPath()+QDir::separator()+DataDefine::CTC_NODE_TEST_EXE,strList);
    }
    else if(2 == _p->chaintype)
    {//正式链
        qDebug() << "start ctc_formal.exe " << strList;
        _p->nodeProc->start(QCoreApplication::applicationDirPath()+QDir::separator()+DataDefine::CTC_NODE_FORMAL_EXE,strList);
    }
}

bool CTCBackStage::exeRunning()
{
    return _p->nodeProc->state() == QProcess::Running && _p->dataRequire->isConnected();
}

QProcess *CTCBackStage::getProcess() const
{
    return _p->nodeProc;
}

void CTCBackStage::ReadyClose()
{
    disconnect(_p->nodeProc,&QProcess::stateChanged,this,&CTCBackStage::onNodeExeStateChanged);
    disconnect(_p->nodeProc,&QProcess::readyReadStandardError,this,&CTCBackStage::readNodeStandError);
    disconnect(_p->nodeProc,&QProcess::readyReadStandardOutput,this,&CTCBackStage::readNodeStandOutput);
    if(exeRunning())
    {
        _p->nodeProc->close();
    }
    else
    {
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

void CTCBackStage::rpcPostedSlot(const QString &cmd, const QString &param)
{
    _p->dataRequire->requirePosted(cmd,param);
}

void CTCBackStage::rpcReceivedSlot(const QString &id, const QString &message)
{
    QString result = message.mid( QString("{\"id\":32800,").size());
    result = result.left( result.size() - 1);
    result.prepend("{");
    result.append("}");
    emit rpcReceived(id,result);
}

void CTCBackStage::onNodeExeStateChanged()
{
    if(_p->nodeProc->state() == QProcess::Starting)
    {
        //qDebug() << QString("%1 is starting").arg("ubcd.exe");
    }
    else if(_p->nodeProc->state() == QProcess::Running)
    {
        qDebug() << QString("ctc %1 is running").arg(_p->chaintype);

        initSocketManager();
        //emit exeStarted();
    }
    else if(_p->nodeProc->state() == QProcess::NotRunning)
    {
        qDebug()<<QString("ctc %1 is notrunning :%2").arg(_p->chaintype).arg(_p->nodeProc->errorString());
//        CommonDialog commonDialog(CommonDialog::OkOnly);
//        commonDialog.setText(tr("Fail to launch ctc %1 !").arg(_p->chaintype));
//        commonDialog.pop();
        emit exeNotRunning();
    }
}

void CTCBackStage::testStartedFinish()
{
    connect(_p->dataRequire,&DataRequireManager::requireResponse,this,&CTCBackStage::testStartReceiveSlot);
    connect(&_p->timerForStartExe,&QTimer::timeout,[this](){
        this->rpcPostedSlot("teststart",IDEUtil::toJsonFormat("blockchain_get_info",QJsonArray()));
    });
    _p->timerForStartExe.start(100);
}

void CTCBackStage::testStartReceiveSlot(const QString &id, const QString &message)
{
    if("teststart" == id && message.contains("blockchain_id"))
    {
        _p->timerForStartExe.stop();
        disconnect(_p->dataRequire,&DataRequireManager::requireResponse,this,&CTCBackStage::testStartReceiveSlot);
        connect(_p->dataRequire,&DataRequireManager::requireResponse,this,&CTCBackStage::rpcReceivedSlot);
        connect(_p->dataRequire,&DataRequireManager::requireOvertime,this,&BackStageBase::rpcOvertime);
        emit exeStarted();
    }
}

void CTCBackStage::readNodeStandError()
{
    QString str = _p->nodeProc->readAllStandardError();
    if(str.isEmpty()) return;
    int chainFlag = static_cast<int>(_p->chaintype==1?DataDefine::NODE_ERROR_TEST_TYPE:DataDefine::NODE_ERROR_FORMAL_TYPE);
    emit AdditionalOutputMessage(str,chainFlag);

}

void CTCBackStage::readNodeStandOutput()
{
    QString str = _p->nodeProc->readAllStandardOutput();
    if(str.isEmpty()) return;
    int chainFlag = static_cast<int>(_p->chaintype==1?DataDefine::NODE_OUT_TEST_TYPE:DataDefine::NODE_OUT_FORMAL_TYPE);
    emit AdditionalOutputMessage(str,chainFlag);

}

void CTCBackStage::initSocketManager()
{
    connect(_p->dataRequire,&DataRequireManager::connectFinish,this,&CTCBackStage::testStartedFinish);
    _p->dataRequire->setHTTPHeaderAdditional("Authorization","Basic YTpi");
    _p->dataRequire->setHTTPPathAdditional("/rpc");
    _p->dataRequire->startManager(DataRequireManager::HTTPWITHUSER);
}
