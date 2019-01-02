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

    static void ParseDebugInfoData(const QString &info,BaseItemDataPtr &root);//解析查询调试器变量返回内容

    static void ParseStackTraceData(const QString &info,ListItemVec &data,const QString &defaultFile="");//解析堆栈信息返回内容
private:
    static bool isCommentLine(const QString &lineInfo,bool &isCommentStart,
                              const QString &lineComment,const QString &phaseCommentStart,const QString &phaseCommentEnd);//辅助判断注释行问题

    static void ParseInfoArrayData(const QJsonArray &arr,BaseItemDataPtr parent);//辅助解析调试器变量查询数组
    static void ParseInfoObjectData(const QJsonObject &obj,BaseItemDataPtr parent);//辅助解析调试器变量查询object
private:
    DebugUtil();
    ~DebugUtil();
};

#endif // DEBUGUTIL_H
