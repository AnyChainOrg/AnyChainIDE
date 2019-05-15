#ifndef DEBUGUTIL_H
#define DEBUGUTIL_H

#include <QString>
#include <vector>
#include <QJsonArray>
#include <QJsonObject>

#include "DebugDataStruct.h"
class DebugUtil
{
public:
    static void getCommentLine(const QString &filePath,std::vector<int> &data);//获取某个文件的注释行、空行

    static bool ParseDebugInfoLocalData(const QString &info,BaseItemDataPtr &root);//解析查询调试器变量返回内容

    static bool ParseDebugInfoUpvalData(const QString &info,BaseItemDataPtr &root);//解析查询调试器返回全局变量内容

    static bool ParseStackTraceData(const QString &info,ListItemVec &data,const QString &defaultFile="");//解析堆栈信息返回内容

    static void ParseBreakPointData(const QString &info,std::tuple<QString,int> &result);

    static bool isPromptFlag(const QString &info);

    static int MakeDebuggerJsonRPC(const QString &method, const QVariantMap &parameters, QString &result);//生成jsonrpc2.0带名请求格式，id自增
    static int MakeDebuggerJsonRPC(const QString &method, const QJsonArray &papameters, QString &result);//生成jsonrpc2.0数组参数的请求格式，id自增
private:
    static bool isCommentLine(const QString &lineInfo,bool &isCommentStart,
                              const QString &lineComment,const QString &phaseCommentStart,const QString &phaseCommentEnd);//辅助判断注释行问题

    static void ParseInfoArrayData(const QJsonArray &arr,BaseItemDataPtr parent);//辅助解析调试器变量查询数组
    static void ParseInfoObjectData(const QJsonObject &obj,BaseItemDataPtr parent);//辅助解析调试器变量查询object

    static QString DoubleToString(double data);
private:
    DebugUtil();
    ~DebugUtil();
};

#endif // DEBUGUTIL_H
