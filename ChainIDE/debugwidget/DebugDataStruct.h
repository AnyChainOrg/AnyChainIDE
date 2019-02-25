#ifndef DEBUGDATASTRUCT_H
#define DEBUGDATASTRUCT_H

#include <QString>
#include <map>
#include <vector>
#include <memory>

//调试数据结构,所有调试需要的数据结构由此定义
namespace DebugDataStruct{
//调试器状态
enum DebuggerState{Available=1, StartDebug,
                   StepDebug, StepInDebug, ContinueDebug,
                   QueryInfo, QueryUpInfo, QueryStack,
                   SetBreakPoint, DeleteBreakPoint};

}

//节点数据结构
class BaseItemData;
typedef std::shared_ptr<BaseItemData> BaseItemDataPtr;
class BaseItemData:std::enable_shared_from_this<BaseItemData>{
public:
    explicit BaseItemData(const QString &k="",const QString &v="",const QString &t="",BaseItemDataPtr p = nullptr)
        :Key(k),Val(v),Type(t),parent(p){
    }
    const QString &getKey()const{return Key;}
    const QString &getVal()const{return Val;}
    const QString &getType()const{return Type;}

    void setKey(const QString &ke){Key = ke;}
    void setVal(const QString &va){Val = va;}
    void setType(const QString &te){Type = te;}

    const std::vector<BaseItemDataPtr> &getChildren()const{return children;}
    void appendChild(BaseItemDataPtr child){children.emplace_back(child);}

    void setParent(BaseItemDataPtr pa){parent = pa;}
    BaseItemDataPtr getParent()const{return parent;}

    void clearData(){children.clear();setKey("");setVal("");setType("");}
private:
    QString Key;
    QString Val;
    QString Type;
    std::vector<BaseItemDataPtr> children;
    BaseItemDataPtr parent;
};
//列表基本数据
class ListItemData:std::enable_shared_from_this<ListItemData>{
public:
    explicit ListItemData(int level=0,const QString &function = "",
                          const QString &file = "",int line=0)
        :Level(level),Function(function),File(file),Line(line)
    {

    }
public:
    void SetLevel(int va){Level = va;}
    int GetLevel()const{return Level;}

    void SetFunction(const QString &va){Function = va;}
    const QString &GetFunction()const{return Function;}

    void SetFile(const QString &va){File = va;}
    const QString &GetFile()const{return File;}

    void SetLine(int va){Line = va;}
    int GetLine()const{return Line;}
private:
    int Level;
    QString Function;
    QString File;
    int Line;
};
typedef std::shared_ptr<ListItemData> ListItemDataPtr;
typedef std::vector<ListItemDataPtr> ListItemVec;
#endif // DEBUGDATASTRUCT_H
