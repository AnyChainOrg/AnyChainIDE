#include "DebugUtil.h"

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


void DebugUtil::ParseDebugInfoLocalData(const QString &info, BaseItemDataPtr &root)
{
    if(nullptr == root)
    {
        root = std::make_shared<BaseItemData>();
    }
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(info.toUtf8(), &json_error);

    if(json_error.error != QJsonParseError::NoError || !parse_doucment.isObject()) return ;

    QJsonArray arr = parse_doucment.object().value("locals").toArray();

    ParseInfoArrayData(arr,root);
}

void DebugUtil::ParseDebugInfoUpvalData(const QString &info, BaseItemDataPtr &root)
{
    if(nullptr == root)
    {
        root = std::make_shared<BaseItemData>();
    }
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(info.toUtf8(), &json_error);

    if(json_error.error != QJsonParseError::NoError || !parse_doucment.isObject()) return ;

    QJsonArray arr = parse_doucment.object().value("upvalues").toArray();

    ParseInfoArrayData(arr,root);
}

void DebugUtil::ParseStackTraceData(const QString &info, ListItemVec &data,const QString &defaultFile)
{
//    QJsonParseError json_error;
//    QJsonDocument parse_doucment = QJsonDocument::fromJson(info.toUtf8(), &json_error);

//    if(json_error.error != QJsonParseError::NoError || !parse_doucment.isObject()) return ;

    data.clear();
    //todo ...test
    QStringList splitResult = info.split("\r\n");
    foreach (QString res, splitResult) {
        if(res.isEmpty()) continue;
        QRegExp rx("^#(\\d+)\\t([\\d\\D]+)\t([\\d\\D]+):([-]*\\d+)$",Qt::CaseInsensitive);
        if(rx.indexIn(res) >= 0 && !rx.cap(1).isEmpty() && !rx.cap(2).isEmpty() &&
           !rx.cap(3).isEmpty() && !rx.cap(4).isEmpty())
        {
            QString debugFile = rx.cap(3);
            if(!defaultFile.isEmpty() && "?" == debugFile)
            {
                data.emplace_back(std::make_shared<ListItemData>(rx.cap(1).toInt(),rx.cap(2),defaultFile,rx.cap(4).toInt()));
            }
            else
            {
                data.emplace_back(std::make_shared<ListItemData>(rx.cap(1).toInt(),rx.cap(2),debugFile,rx.cap(4).toInt()));
            }
        }
    }
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

DebugUtil::DebugUtil()
{
}

DebugUtil::~DebugUtil()
{

}
