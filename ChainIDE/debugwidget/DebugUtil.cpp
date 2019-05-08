#include "DebugUtil.h"

#include <atomic>
#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include <QStringList>
#include "DataDefine.h"

void DebugUtil::getCommentLine(const QString &filePath, std::vector<int> &data)
{
    data.clear();
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    //设置注释风格问题
    QString lineComment("//"),phaseStart("/*"),phaseEnd("*/");
    if(filePath.endsWith("."+DataDefine::GLUA_SUFFIX))
    {
        //lua语言的注释，以--或者[[开头，]]结尾
        lineComment = "--";
        phaseStart = "--[[";
        phaseEnd = "]]";
    }

    bool isCommentStart = false;//标识是否已经进入注释段落
    int currentLine = 0;
    while (!file.atEnd())
    {
        QString line(file.readLine());
        line = line.trimmed();

        if(isCommentStart)
        {
            data.emplace_back(currentLine);
            isCommentLine(line,isCommentStart,lineComment,phaseStart,phaseEnd);
        }
        else if(isCommentLine(line,isCommentStart,lineComment,phaseStart,phaseEnd))
        {
            data.emplace_back(currentLine);
        }

        ++currentLine;
    }
}

bool DebugUtil::isCommentLine(const QString &lineInfo, bool &isCommentStart,
                              const QString &lineComment,const QString &phaseCommentStart,const QString &phaseCommentEnd)
{
    if(lineInfo.isEmpty())
    {
        return true;
    }
    else if(lineInfo.startsWith(phaseCommentEnd) && lineInfo.length() == phaseCommentEnd.length())
    {
        isCommentStart = false;
        return true;
    }
    else if(lineInfo.startsWith(phaseCommentStart))
    {
        if(lineInfo.length() == phaseCommentStart.length() || !lineInfo.endsWith(phaseCommentEnd))
        {
            return isCommentStart = true;
        }
    }
    else if(lineInfo.startsWith(lineComment))
    {
        return true;
    }


    int index = lineInfo.indexOf(phaseCommentStart);
    if(-1 != index)
    {
        isCommentStart = true;
    }

    if(isCommentStart)
    {
        index = lineInfo.indexOf(phaseCommentEnd);
        if (-1 != index)
        {
            isCommentStart = false;
            if (isCommentLine(lineInfo.mid(phaseCommentEnd.length()).trimmed(), isCommentStart,lineComment,phaseCommentStart,phaseCommentEnd))
            {
                return true;
            }
        }
    }
    return false;
}


bool DebugUtil::ParseDebugInfoLocalData(const QString &info, BaseItemDataPtr &root)
{
    if(nullptr == root)
    {
        root = std::make_shared<BaseItemData>();
    }
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(info.toUtf8(), &json_error);

    if(json_error.error != QJsonParseError::NoError || !parse_doucment.isObject()) return false;

    if(!parse_doucment.object().value("result").toObject().keys().contains("locals"))
    {
       return false;
    }

    QJsonArray arr = parse_doucment.object().value("result").toObject().value("locals").toArray();

    ParseInfoArrayData(arr,root);
    return true;
}

bool DebugUtil::ParseDebugInfoUpvalData(const QString &info, BaseItemDataPtr &root)
{
    if(nullptr == root)
    {
        root = std::make_shared<BaseItemData>();
    }
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(info.toUtf8(), &json_error);

    if(json_error.error != QJsonParseError::NoError || !parse_doucment.isObject()) return false;

    if(!parse_doucment.object().value("result").toObject().keys().contains("upvalues"))
    {
        return false;
    }
    QJsonArray arr = parse_doucment.object().value("result").toObject().value("upvalues").toArray();

    ParseInfoArrayData(arr,root);
    return true;
}

bool DebugUtil::ParseStackTraceData(const QString &info, ListItemVec &data,const QString &defaultFile)
{
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(info.toUtf8(), &json_error);

    if(json_error.error != QJsonParseError::NoError || !parse_doucment.isObject()) return false;

    if(!parse_doucment.object().value("result").toObject().keys().contains("backtrace"))
    {
        return false;
    }

    data.clear();

    QJsonArray arr = parse_doucment.object().value("result").toObject().value("backtrace").toArray();
    foreach (QJsonValue val, arr) {
        if(!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        int level = obj.value("FrameId").toInt();
        QJsonObject detail = obj.value("Detail").toObject();
        int line = detail.value("currline").toInt();
        QString function = detail.value("name").isNull()?"null":detail.value("name").toString();
        QString debugFile = detail.value("short_src").toString();
        if(!defaultFile.isEmpty() && "?" == debugFile)
        {
            debugFile = defaultFile;
        }
        data.emplace_back(std::make_shared<ListItemData>(level,function,debugFile,line));
    }
    return true;
}

void DebugUtil::ParseInfoArrayData(const QJsonArray &arr, BaseItemDataPtr parent)
{
    if(!parent) return;
    foreach (QJsonValue val, arr) {
        if(!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        BaseItemDataPtr data = std::make_shared<BaseItemData>(obj.value("valName").toString(),"",obj.value("valType").toString(),parent);
        parent->appendChild(data);

        QJsonValue typeVal = obj.value("valDetails");
        if(typeVal.isString())
        {
            data->setVal(typeVal.toString());
        }
        else if(typeVal.isBool())
        {
            data->setVal(typeVal.toBool()?"true":"false");
        }
        else if(typeVal.isDouble())
        {
            data->setVal(QString::number(typeVal.toDouble()));
        }
        else if(typeVal.isObject())
        {
            ParseInfoObjectData(typeVal.toObject(),data);
        }
        else if(typeVal.isArray())
        {
            ParseInfoArrayData(typeVal.toArray(),data);
        }
    }
}

void DebugUtil::ParseInfoObjectData(const QJsonObject &obj, BaseItemDataPtr parent)
{
    if(!parent) return;
    QJsonObject::Iterator it;
    for(auto it=obj.begin();it!=obj.end();it++)
    {
        BaseItemDataPtr data = std::make_shared<BaseItemData>(it.key(),"","");
        parent->appendChild(data);
        QJsonValue val = it.value();
        if(val.isString())
        {
            data->setVal(val.toString());
            data->setType("string");
        }
        else if(val.isDouble())
        {
            data->setVal(QString::number(val.toDouble()));
            data->setType("double");
        }
        else if(val.isBool())
        {
            data->setVal(val.toBool()?"true":"false");
            data->setType("bool");
        }
        else if(val.isArray())
        {
            ParseInfoArrayData(val.toArray(),data);
            data->setType("array");
        }
        else if(val.isObject())
        {
            ParseInfoObjectData(val.toObject(),data);
            data->setType("object");
        }
    }
}

void DebugUtil::ParseBreakPointData(const QString &info,std::tuple<QString,int> &result)
{
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(info.toUtf8(), &json_error);

    if(json_error.error != QJsonParseError::NoError || !parse_doucment.isObject()) return ;
    if("Notify" == parse_doucment.object().value("method").toString())
    {
        QJsonObject obj = parse_doucment.object().value("params").toObject();
        if("breakpoint hit" == obj.value("Event").toString() )
        {
            result =  std::make_tuple(obj.value("File").toString(),obj.value("LineNo").toInt());
        }
    }

    //  非jsonrpc版本
//    QString data = info.simplified();
//    QRegExp rx("hit breakpoint at (.*):(\\d+)",Qt::CaseInsensitive);
//    rx.indexIn(data);
//    if(rx.indexIn(data) < 0 || rx.cap(1).isEmpty() || rx.cap(2).isEmpty()) return;
//        result =  std::make_tuple("",rx.cap(2).toInt());
}

bool DebugUtil::isPromptFlag(const QString &info)
{
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(info.toUtf8(), &json_error);
    if(json_error.error != QJsonParseError::NoError || !parse_doucment.isObject()) return false;
    if("Notify" == parse_doucment.object().value("method").toString())
    {
        QJsonObject obj = parse_doucment.object().value("params").toObject();
        return "prompt" == obj.value("Event").toString();
    }
    return false;
}

static std::atomic<int> initID(1);
int DebugUtil::MakeDebuggerJsonRPC(const QString &method, const QVariantMap &parameters, QString &result)
{
    int postID = initID.fetch_add(1);

    QJsonObject object;
    object.insert("jsonrpc","2.0");
    object.insert("id",postID);
    object.insert("method",method);
    object.insert("params",QJsonObject::fromVariantMap(parameters));
    result = QJsonDocument(object).toJson();
    return  postID;
}

int DebugUtil::MakeDebuggerJsonRPC(const QString &method, const QJsonArray &parameters, QString &result)
{
    int postID = initID.fetch_add(1);

    QJsonObject object;
    object.insert("jsonrpc","2.0");
    object.insert("id",postID);
    object.insert("method",method);
    object.insert("params",parameters);
    result = QString(QJsonDocument(object).toJson()).simplified().append("\n");
    return  postID;

    //非jsonrpc版本
//    result = method;
//    foreach (QJsonValue va, parameters) {
//        result += " " + va.toString();
//    }
//    result +="\n";
//    return 0;
}

DebugUtil::DebugUtil()
{
}

DebugUtil::~DebugUtil()
{

}
