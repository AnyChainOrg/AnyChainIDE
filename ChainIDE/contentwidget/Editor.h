#ifndef EDITOR_H
#define EDITOR_H

#include <QWidget>

namespace Ui {
class Editor;
}
class QWebEnginePage;
//编辑器基类
class Editor : public QWidget
{
    Q_OBJECT
public:
    explicit Editor(const QString &path,const QString &htmlPath = "",QWidget *parent = nullptr);
    virtual ~Editor();
signals:
    void stateChanged();
public:
    virtual void checkState() = 0;

    virtual void setText(QString text) = 0;
    virtual QString getText() = 0;

    virtual QString getSelectedText() = 0;

    virtual void SetBreakPoint(int line);//强制添加某一行为断点
    virtual void RemoveBreakPoint(int lint);//强制删除某一行断点
    virtual void TabBreakPoint();//自动切换当前行断点方式
    virtual void ClearBreakPoint();
    const std::vector<int> &getBreakPoints()const;//已升序排序
    void setBeakPoint(int line,bool isAdd);//断点变量自动更新

    virtual void SetDebuggerLine(int lineNumber);//设置并自动更新其他断点样式、跳转到该行
    virtual void ClearDebuggerLine();//清空所有编辑器的断点样式

    bool isUndoAvailable();//判断当前是否可撤销
    bool isRedoAvailable();//判断当前是否可重写

    bool isEditable()const;
    virtual void setEditable(bool);

    void setSaved(bool isSaved) ;
    bool isSaved()const;

    bool saveFile() ;//保存文件
    const QString &getFilePath()const;//获取文件路径

    virtual void JumpToLine(int line,int ch);//跳转到某行某个位置
protected:
    void setUndoAvaliable(bool);
    void setRedoAvaliable(bool);
public slots:
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual void cut() = 0;
    virtual void copy() = 0;
    virtual void paste() = 0;
    virtual void deleteText() = 0;
    virtual void selectAll() = 0;
protected:
    void InitEditor();
    void openFile(const QString &filePath);

protected:
    bool eventFilter(QObject *watched, QEvent *e);
    QWebEnginePage *getPage()const;
    QVariant syncRunJavaScript(const QString &javascript, int msec = 3000);
    void RunJavaScript(const QString &scriptSource);
private:
    Ui::Editor *ui;
    class DataPrivate;
    DataPrivate *_p;
};

#endif // EDITOR_H
