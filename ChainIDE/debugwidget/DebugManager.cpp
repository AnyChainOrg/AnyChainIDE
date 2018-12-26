#include "DebugManager.h"

#include <mutex>
#include <algorithm>

#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
#include <QDebug>
#include <QTimer>

#include "DataDefine.h"
#include "DebugUtil.h"

class DebugManager::DataPrivate
{
public:
    DataPrivate()
        :uvmProcess(new QProcess())
        ,debuggerState(DebugDataStruct::Available)
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
    std::mutex breakMutex;

    QProcess *uvmProcess;
    DebugDataStruct::DebuggerState debuggerState;

    std::mutex dataMutex;
};

DebugManager::DebugManager(QObject *parent)
    : QObject(parent)
    ,_p(new DataPrivate())
{
    InitDebugger();
}

DebugManager::~DebugManager()
{
    qDebug()<<"delete debugMmanager";
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
    params<<"-x"<<"-luvmdebug"<<"-k"<<_p->outFilePath<<api<<param;

    qDebug()<<"start debug"<<QCoreApplication::applicationDirPath()+"/"+DataDefine::DEBUGGER_UVM_DIR+"/"+DataDefine::DEBUGGER_UVM_NAME<<params;
    _p->uvmProcess->start(QCoreApplication::applicationDirPath()+"/"+DataDefine::DEBUGGER_UVM_DIR+"/"+DataDefine::DEBUGGER_UVM_NAME,params);

}

void DebugManager::debugNextStep()
{
    setDebuggerState(DebugDataStruct::StepDebug);
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
    _p->uvmProcess->write("continue\n");
    _p->uvmProcess->waitForFinished();
    _p->uvmProcess->close();
    ResetDebugger();
    emit debugFinish();
}

void DebugManager::getVariantInfo()
{
    setDebuggerState(DebugDataStruct::QueryInfo);
    _p->uvmProcess->write("info locals\n");
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
        _p->uvmProcess->write("step\n");
        break;
    case DebugDataStruct::ContinueDebug:
        //调整一下当前文件的断点，主要先防止注释行之类的断点
        UpdateFileDebugBreak(data);
        _p->uvmProcess->write("continue\n");
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
    _p->uvmProcess->close();
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

    switch(getDebuggerState()){
    case DebugDataStruct::QueryInfo:
        ParseQueryInfo(outPut);
        break;
    case DebugDataStruct::QueryStack:
        break;
    default:
        ParseBreakPoint(outPut);
        emit debugOutput(outPut);
        break;
    }
}

void DebugManager::readyReadStandardErrorSlot()
{
    emit debugOutput(_p->uvmProcess->readAllStandardError());
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
}

void DebugManager::ResetDebugger()
{
    //设置uvm工作目录为当前uvm_single.exe所在目录
    _p->uvmProcess->setWorkingDirectory(QCoreApplication::applicationDirPath()+"/"+DataDefine::DEBUGGER_UVM_DIR);
    setDebuggerState(DebugDataStruct::Available);
    _p->breakPointLines.clear();
    _p->commentLines.clear();
}

void DebugManager::ParseQueryInfo(const QString &info)
{
    BaseItemDataPtr root = std::make_shared<BaseItemData>();

    DebugUtil::ParseDebugData(info,root);

    emit showVariant(root);
}

void DebugManager::ParseBreakPoint(const QString &info)
{
    QString data = info.simplified();
    QRegExp rx("hit breakpoint at (.*):(\\d+)",Qt::CaseInsensitive);
    rx.indexIn(data);
    if(rx.indexIn(data) < 0 || rx.cap(1).isEmpty() || rx.cap(2).isEmpty()) return;
    emit debugBreakAt(_p->filePath,rx.cap(2).toInt()-1);
    getVariantInfo();

}

void DebugManager::SetBreakPoint(const QString &file, int lineNumber)
{
    setDebuggerState(DebugDataStruct::SetBreakPoint);
    _p->uvmProcess->write(QString("break ? %1\n").arg(QString::number(lineNumber+1)).toStdString().c_str());
    _p->uvmProcess->waitForBytesWritten();
}

void DebugManager::DelBreakPoint(const QString &file, int lineNumber)
{
    setDebuggerState(DebugDataStruct::DeleteBreakPoint);
    _p->uvmProcess->write(QString("delete ? %1\n").arg(QString::number(lineNumber+1)).toStdString().c_str());
    _p->uvmProcess->waitForBytesWritten();
}

void DebugManager::CancelBreakPoint()
{
    std::for_each(_p->breakPointLines.begin(),_p->breakPointLines.end(),std::bind(&DebugManager::DelBreakPoint,this,_p->filePath,std::placeholders::_1));
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
    //调整断点情况
    result.clear();
    std::vector<int> temp = data;
    std::for_each(temp.begin(),temp.end(),[this,&result](int li){
        if(this->_p->commentLines.end() != std::find(this->_p->commentLines.begin(),this->_p->commentLines.end(),li))
        {
            //如果断点在注释行中，就删除，并且顺移到下一个非注释行
            emit removeBreakPoint(this->_p->filePath,li);
            int line = li;
            while(this->_p->commentLines.end() != std::find(this->_p->commentLines.begin(),this->_p->commentLines.end(),line))
            {
                ++line;
            }
            result.emplace_back(line);
            emit addBreakPoint(this->_p->filePath,line);
        }
        else
        {
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
    std::for_each(deleteVec.begin(),deleteVec.end(),std::bind(&DebugManager::DelBreakPoint,this,_p->filePath,std::placeholders::_1));
    std::for_each(addVec.begin(),addVec.end(),std::bind(&DebugManager::SetBreakPoint,this,_p->filePath,std::placeholders::_1));
}
