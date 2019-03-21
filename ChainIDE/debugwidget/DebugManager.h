#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H

#include "DebugDataStruct.h"
#include <QObject>
//调试管理
class DebugManager : public QObject
{
    Q_OBJECT
public:
    explicit DebugManager(QObject *parent = nullptr);
    ~DebugManager();
public:
    //开始调试，源码文件，out文件，函数名，参数
    void startDebug(const QString &sourceFilePath,const QString &byteFilePath,const QString &api,const QStringList &param);

    void debugNextStep();//调试到下一行
    void debugStepInto();//调试到函数内部
    void debugContinue();//调试到下一个断点
    void stopDebug();//停止调试

    DebugDataStruct::DebuggerState getDebuggerState()const;//获取调试器状态

    void ReadyClose();//准备关闭

    const QString &getCurrentDebugFile()const;//获取当前调试文件

public slots:
    void fetchBreakPointsFinish(const QString &filePath,const std::vector<int> &data);

private slots:
    //调试器的一些槽、状态变化、对外输出、对外错误
    void OnProcessStateChanged();
    void readyReadStandardOutputSlot();
    void readyReadStandardErrorSlot();
    //调试器网络传输数据接收
    void readSocketData(const QString &data);
private:
    void InitDebugger();//初始化调试器
    void ResetDebugger();//重置调试器，主要重置调试器状态以及一些变量等

    void setDebuggerState(DebugDataStruct::DebuggerState state);//设置调试器状态

    void getVariantInfo();//获取变量信息
    bool ParseInfoLocals(const QString &info);//解析查询变量返回情况

    void getUpValueVariantInfo();//获取全局变量信息
    bool ParseInfoUpValue(const QString &info);//解析查询全局变量返回情况

    void getBackTraceInfo();//获取堆栈信息
    bool ParseBackTrace(const QString &info);//解析查询堆栈返回情况

    void ParseBreakPoint(const QString &info);//解析断点停顿信息

    void SetBreakPoint(const QString &file,int lineNumber);//设置一次断点--调试器
    void DelBreakPoint(const QString &file,int lineNumber);//删除一个断点--调试器
    void CancelBreakPoint();//清除断点--调试器

    //调整断点位置，取消注释行的断点
    void UpdateFileDebugBreak(const std::vector<int> &data);
    void ModifyBreakPoint(const std::vector<int> &data,std::vector<int> &result);
    void UpdateDebuggerBreakCMD(const std::vector<int> &oldBreak,const std::vector<int> &newBreak);
signals:
    void fetchBreakPoints(const QString &filePath);//请求查询断点信息
    void debugOutput(const QString &message);//输出
    void debugStarted();//调试器开始进入调试状态
    void debugFinish();//调试完成
    void debugBreakAt(const QString &file,int lineNumber);//断点停顿，用于告诉外界，刷新断点显示
    void debugError();//调试出错

    void variantUpdated(BaseItemDataPtr data);//变量内容更新
    void backTraceUpdated(const ListItemVec &data);//堆栈内容更新

    void removeBreakPoint(const QString &file,int linenumber);//强制删除断点--注释行
    void addBreakPoint(const QString &file,int linenumber);//强制添加断点--注释行断点的下一个非注释行
private:
    //统一管理发送给调试器的命令
    QString getCommandStr(DebugDataStruct::DebuggerState state)const;//用于获取调试命令模板
    void postCommandToDebugger(const QString &command);//发送调试指令到调试器。所有的指令都由此发送
private:
    class DataPrivate;
    DataPrivate *_p;
};

#endif // DEBUGMANAGER_H
