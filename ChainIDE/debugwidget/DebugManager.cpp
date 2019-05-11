#include "DebugManager.h"

#include <mutex>
#include <algorithm>

#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
#include <QDebug>
#include <QTimer>
#include <QProcess>

#include "DataDefine.h"
#include "DebugUtil.h"
#include "DebuggerDataReuqire.h"

static const QString DebuggerIp="127.0.0.1";
static const QString DebuggerPort="3563";
class DebugManager::DataPrivate
{
public:
    DataPrivate()
        :uvmProcess(new QProcess())
        ,debuggerState(DebugDataStruct::Available)
        ,debuggerTCP(new DebuggerDataReuqire())
        ,infoRootData(std::make_shared<BaseItemData>())
    {

    }
    ~DataPrivate()
    {
        delete uvmProcess;
        uvmProcess = nullptr;
    }
public:
    QString filePath;//当前调试文件
    QString outFilePath;//。out字节码文件
    std::vector<int> commentLines;//当前文件的注释行
    std::vector<int> breakPointLines;//当前文件的断点行

    QProcess *uvmProcess;//调试器
    DebugDataStruct::DebuggerState debuggerState;//调试器当前状态
    std::mutex dataMutex;//用于修改调试器状态的锁
    DebuggerDataReuqire *debuggerTCP;//调试器网络接口

    BaseItemDataPtr infoRootData;//变量查询数据

};

DebugManager::DebugManager(QObject *parent)
    : QObject(parent)
    ,_p(new DataPrivate())
{
    InitDebugger();
}

DebugManager::~DebugManager()
{
    //qDebug()<<"delete debugMmanager";
    delete _p;
    _p = nullptr;
}

void DebugManager::startDebug(const QString &sourceFilePath,const QString &byteFilePath,
                              const QString &api,const QStringList &param)
{
    ResetDebugger();
    _p->filePath = sourceFilePath;
    _p->outFilePath = byteFilePath;
    //源码判定
    if(!QFileInfo(_p->filePath).isFile() || !QFileInfo(_p->filePath).exists())
    {
        emit debugOutput(_p->filePath+" isn't a file or exists");
        emit debugError();
        return ;
    }
    //.out字节码判定
    if(!QFileInfo(_p->outFilePath).isFile() || !QFileInfo(_p->outFilePath).exists())
    {
        emit debugOutput(_p->outFilePath +" isn't a file or exists");
        emit debugError();
        return ;
    }

    //计算当前文件的注释行，用于调整断点
    DebugUtil::getCommentLine(_p->filePath,_p->commentLines);

    //启动单步调试器
    QStringList params;
#ifdef Win32
    params<<"-x"<<"-luvmdebug"<<"-k"<<_p->outFilePath<<api<<param;
#else
    params<<"-x"<<"-k"<<_p->outFilePath<<api<<param;
#endif
    qDebug()<<"start debug"<<QCoreApplication::applicationDirPath()+"/"+DataDefine::DEBUGGER_UVM_DIR+"/"+DataDefine::DEBUGGER_UVM_NAME<<params;
    _p->uvmProcess->start(QCoreApplication::applicationDirPath()+"/"+DataDefine::DEBUGGER_UVM_DIR+"/"+DataDefine::DEBUGGER_UVM_NAME,params);

}

void DebugManager::debugNextStep()
{
    setDebuggerState(DebugDataStruct::StepDebug);
    emit fetchBreakPoints(_p->filePath);
}

void DebugManager::debugStepInto()
{
    setDebuggerState(DebugDataStruct::StepInDebug);
    emit fetchBreakPoints(_p->filePath);
}

void DebugManager::debugContinue()
{
    setDebuggerState(DebugDataStruct::ContinueDebug);
    emit fetchBreakPoints(_p->filePath);
}

void DebugManager::stopDebug()
{
    CancelBreakPoint();
    postCommandToDebugger(getCommandStr(DebugDataStruct::ContinueDebug));
    _p->uvmProcess->waitForFinished();
    _p->uvmProcess->close();
    ResetDebugger();
    emit debugFinish();
}

void DebugManager::fetchBreakPointsFinish(const QString &filePath,const std::vector<int> &data)
{
    switch (getDebuggerState()) {
    case DebugDataStruct::StartDebug:
        //获取到文件的断点信息
        emit debugStarted();
        //直接进行调试，因为默认会停在第一行
        debugContinue();
        break;
    case DebugDataStruct::StepDebug:
        postCommandToDebugger(getCommandStr(DebugDataStruct::StepDebug));
        break;
    case DebugDataStruct::StepInDebug:
        postCommandToDebugger(getCommandStr(DebugDataStruct::StepInDebug));
        break;
    case DebugDataStruct::ContinueDebug:
        //调整一下当前文件的断点，主要先防止注释行之类的断点
        UpdateFileDebugBreak(data);
        postCommandToDebugger(getCommandStr(DebugDataStruct::ContinueDebug));
        break;
    default:
        break;
    }

}

DebugDataStruct::DebuggerState DebugManager::getDebuggerState() const
{
    std::lock_guard<std::mutex> loc(_p->dataMutex);
    return _p->debuggerState;
}

void DebugManager::setDebuggerState(DebugDataStruct::DebuggerState state)
{
    std::lock_guard<std::mutex> loc(_p->dataMutex);
    _p->debuggerState = state;
}

void DebugManager::ReadyClose()
{
    if(_p->uvmProcess->state() == QProcess::Running)
    {
        stopDebug();
    }
}

const QString &DebugManager::getCurrentDebugFile() const
{
    return _p->filePath;
}

void DebugManager::OnProcessStateChanged()
{
    switch (_p->uvmProcess->state()) {
    case QProcess::Starting:
        break;
    case QProcess::Running:
        //先建立socket链接
        _p->debuggerTCP->startConnect(DebuggerIp,DebuggerPort);
        if(!_p->debuggerTCP->isConnected()) break;
        //设置调试器状态
        setDebuggerState(DebugDataStruct::StartDebug);
        //获取当前文件所有断点
        emit fetchBreakPoints(_p->filePath);
        break;
    case QProcess::NotRunning:
        qDebug()<<"debugger not running";
        ResetDebugger();
        emit debugFinish();
        break;
    default:
        break;
    }
}

void DebugManager::readyReadStandardOutputSlot()
{
    QString outPut = QString::fromLocal8Bit( _p->uvmProcess->readAllStandardOutput());
    if(outPut.trimmed().isEmpty())
    {
        return;
    }
////    qDebug()<<"standard output:"<<outPut<<getDebuggerState();
//    if(DebugUtil::isPromptFlag(outPut))
//    {
//        return;
//    }
//    switch(getDebuggerState()){
//    case DebugDataStruct::QueryInfo:
//        //获取info栈变量之后，立马获取全局堆变量
//        if(ParseInfoLocals(outPut))
//        {
//            getUpValueVariantInfo();
//        }
//        break;
//    case DebugDataStruct::QueryUpInfo:
//        //获取全局变量之后，立马获取堆栈回溯状态
//        if(ParseInfoUpValue(outPut))
//        {
//            getBackTraceInfo();
//        }
//        break;
//    case DebugDataStruct::QueryStack:
//        ParseBackTrace(outPut);
//        break;
//    default:
//        //可能是主动推送，默认情况尝试解析断点停顿，并且输出到前台
//        ParseBreakPoint(outPut);
//        emit debugOutput(outPut);
//        break;
//    }
    emit debugOutput(outPut);
//    qDebug()<<"standard output:"<<outPut;
}

void DebugManager::readyReadStandardErrorSlot()
{
    QString errPut = QString::fromLocal8Bit(_p->uvmProcess->readAllStandardError());
    if(errPut.trimmed().isEmpty())
    {
        return;
    }
    emit debugOutput(errPut);
    qDebug()<<"error output:"<<errPut;
}

void DebugManager::readSocketData(const QString &data)
{
    QString outPut(data);
//    qDebug()<<data;
    if(outPut.trimmed().isEmpty())
    {
        return;
    }
    if(DebugUtil::isPromptFlag(outPut))
    {
        return;
    }
    switch(getDebuggerState()){
    case DebugDataStruct::QueryInfo:
        //获取info栈变量之后，立马获取全局堆变量
        if(ParseInfoLocals(outPut))
        {
            getUpValueVariantInfo();
        }
        break;
    case DebugDataStruct::QueryUpInfo:
        //获取全局变量之后，立马获取堆栈回溯状态
        if(ParseInfoUpValue(outPut))
        {
            getBackTraceInfo();
        }
        break;
    case DebugDataStruct::QueryStack:
        ParseBackTrace(outPut);
        break;
    default:
        //可能是主动推送，默认情况尝试解析断点停顿，并且输出到前台
        ParseBreakPoint(outPut);
        break;
    }
//    emit debugOutput(outPut);
//    qDebug()<<"receive from debugger:"<<outPut;

}

void DebugManager::InitDebugger()
{
    connect(_p->uvmProcess,&QProcess::stateChanged,this,&DebugManager::OnProcessStateChanged);
    connect(_p->uvmProcess,&QProcess::readyReadStandardOutput,this,&DebugManager::readyReadStandardOutputSlot);
    connect(_p->uvmProcess,&QProcess::readyReadStandardError,this,&DebugManager::readyReadStandardErrorSlot);
    connect(_p->uvmProcess,static_cast<void (QProcess::*)(QProcess::ProcessError)>( &QProcess::error),[this](QProcess::ProcessError error){
        emit debugOutput( this->_p->uvmProcess->errorString());
        emit debugError();
    });
    connect(_p->debuggerTCP,&DebuggerDataReuqire::receiveData,this,&DebugManager::readSocketData);
}

void DebugManager::ResetDebugger()
{
    //设置uvm工作目录为当前uvm_single.exe所在目录
    _p->uvmProcess->setWorkingDirectory(QCoreApplication::applicationDirPath()+"/"+DataDefine::DEBUGGER_UVM_DIR);
    setDebuggerState(DebugDataStruct::Available);
    _p->breakPointLines.clear();
    _p->commentLines.clear();
}

void DebugManager::getVariantInfo()
{
    setDebuggerState(DebugDataStruct::QueryInfo);
    postCommandToDebugger(getCommandStr(DebugDataStruct::QueryInfo));
}

void DebugManager::getUpValueVariantInfo()
{
    setDebuggerState(DebugDataStruct::QueryUpInfo);
    postCommandToDebugger(getCommandStr(DebugDataStruct::QueryUpInfo));
}

void DebugManager::getBackTraceInfo()
{
    setDebuggerState(DebugDataStruct::QueryStack);
    postCommandToDebugger(getCommandStr(DebugDataStruct::QueryStack));
}

bool DebugManager::ParseInfoLocals(const QString &info)
{
    _p->infoRootData->clearData();
    if(DebugUtil::ParseDebugInfoLocalData(info,_p->infoRootData))
    {
        emit variantUpdated(_p->infoRootData);
        return true;
    }
    return false;
}

bool DebugManager::ParseInfoUpValue(const QString &info)
{
    if(DebugUtil::ParseDebugInfoUpvalData(info,_p->infoRootData))
    {
        emit variantUpdated(_p->infoRootData);
        return true;
    }
    return false;
}

bool DebugManager::ParseBackTrace(const QString &info)
{
    ListItemVec data;
    if(DebugUtil::ParseStackTraceData(info,data,QFileInfo(_p->filePath).fileName()))
    {
        emit backTraceUpdated(data);
        return true;
    }
    return false;

}

void DebugManager::ParseBreakPoint(const QString &info)
{
    std::tuple<QString,int> data = std::make_tuple("",-1);
    DebugUtil::ParseBreakPointData(info,data);
    if(std::get<1>(data) != -1)
    {
        //如果判断为断点停顿，，发送停顿消息，发送查询变量消息
        emit debugBreakAt(_p->filePath,std::get<1>(data)-1);
        getVariantInfo();
    }
}

void DebugManager::SetBreakPoint(const QString &file, int lineNumber)
{
    setDebuggerState(DebugDataStruct::SetBreakPoint);
    postCommandToDebugger(getCommandStr(DebugDataStruct::SetBreakPoint).arg("?").arg(QString::number(lineNumber+1)));
}

void DebugManager::DelBreakPoint(const QString &file, int lineNumber)
{
    setDebuggerState(DebugDataStruct::DeleteBreakPoint);
    postCommandToDebugger(getCommandStr(DebugDataStruct::DeleteBreakPoint).arg("?").arg(QString::number(lineNumber+1)));
}

void DebugManager::CancelBreakPoint()
{
    //重复调用删除断点函数即可
    std::for_each(_p->breakPointLines.begin(),_p->breakPointLines.end(),std::bind(&DebugManager::DelBreakPoint,this,std::ref(_p->filePath),std::placeholders::_1));
}

void DebugManager::UpdateFileDebugBreak(const std::vector<int> &data)
{
    //获取并调整真正的断点
    std::vector<int> modifyBreakpoints;
    ModifyBreakPoint(data,modifyBreakpoints);
    //向调试器发送调整断点命令
    UpdateDebuggerBreakCMD(_p->breakPointLines,modifyBreakpoints);
    //修改缓存断点信息
    _p->breakPointLines = modifyBreakpoints;
}

void DebugManager::ModifyBreakPoint(const std::vector<int> &data,std::vector<int> &result)
{
    //调整断点情况，主要判断是否在注释行打了断点，，其实可不必，因为调试器会忽略这些断点
    result.clear();
    std::vector<int> temp = data;
    std::for_each(temp.begin(),temp.end(),[this,&result](int li){
        if(this->_p->commentLines.end() != std::find(this->_p->commentLines.begin(),this->_p->commentLines.end(),li)){
            //如果断点在注释行中，就删除，并且顺移到下一个非注释行
            emit removeBreakPoint(this->_p->filePath,li);
            int line = li;
            while(this->_p->commentLines.end() != std::find(this->_p->commentLines.begin(),this->_p->commentLines.end(),line)){
                ++line;
            }
            result.emplace_back(line);
            emit addBreakPoint(this->_p->filePath,line);
        }
        else{
            result.emplace_back(li);
        }
    });
}

void DebugManager::UpdateDebuggerBreakCMD(const std::vector<int> &oldBreak, const std::vector<int> &newBreak)
{
    std::vector<int> deleteVec;
    std::set_difference(oldBreak.begin(),oldBreak.end(),newBreak.begin(),newBreak.end(),std::inserter(deleteVec,deleteVec.begin()));

    std::vector<int> addVec;
    std::set_difference(newBreak.begin(),newBreak.end(),oldBreak.begin(),oldBreak.end(),std::inserter(addVec,addVec.begin()));

    //删除已经没有的断点，添加新的断点
    std::for_each(deleteVec.begin(),deleteVec.end(),std::bind(&DebugManager::DelBreakPoint,this,std::ref(_p->filePath),std::placeholders::_1));
    std::for_each(addVec.begin(),addVec.end(),std::bind(&DebugManager::SetBreakPoint,this,std::ref(_p->filePath),std::placeholders::_1));
}

QString DebugManager::getCommandStr(DebugDataStruct::DebuggerState state) const
{
    QString command("");
    switch (state) {
    case DebugDataStruct::ContinueDebug:
        DebugUtil::MakeDebuggerJsonRPC("continue",QJsonArray(),command);
        break;
    case DebugDataStruct::StepDebug:
        DebugUtil::MakeDebuggerJsonRPC("step",QJsonArray(),command);
        break;
    case DebugDataStruct::StepInDebug:
        DebugUtil::MakeDebuggerJsonRPC("next",QJsonArray(),command);
        break;
    case DebugDataStruct::QueryInfo:
        DebugUtil::MakeDebuggerJsonRPC("info",QJsonArray()<<"locals",command);
        break;
    case DebugDataStruct::QueryUpInfo:
        DebugUtil::MakeDebuggerJsonRPC("info",QJsonArray()<<"upvalues",command);
        break;
    case DebugDataStruct::QueryStack:
        DebugUtil::MakeDebuggerJsonRPC("backtrace",QJsonArray(),command);
        break;
    case DebugDataStruct::SetBreakPoint:
        DebugUtil::MakeDebuggerJsonRPC("break",QJsonArray()<<"%1"<<"%2",command);
        break;
    case DebugDataStruct::DeleteBreakPoint:
        DebugUtil::MakeDebuggerJsonRPC("delete",QJsonArray()<<"%1"<<"%2",command);
        break;
    default:
        break;
    }
    return command;
}

void DebugManager::postCommandToDebugger(const QString &command)
{
//    qDebug()<<"send to debugger:"<<command;
//    _p->uvmProcess->write(command.toStdString().c_str());
//    _p->uvmProcess->waitForBytesWritten();
    //发送给网络端口
    _p->debuggerTCP->postData(command);
}
