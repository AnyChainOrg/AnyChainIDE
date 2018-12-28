#ifndef DEBUGWIDGET_H
#define DEBUGWIDGET_H

#include <memory>
#include <QWidget>

class QTableWidgetItem;
namespace Ui {
class DebugWidget;
}

class BaseItemData;
typedef std::shared_ptr<BaseItemData> BaseItemDataPtr;

class ListItemData;
typedef std::shared_ptr<ListItemData> ListItemDataPtr;
typedef std::vector<ListItemDataPtr> ListItemVec;

//调试窗口
class DebugWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DebugWidget(QWidget *parent = nullptr);
    ~DebugWidget();
signals:
    void JumpToLine(int line);
public:
    void UpdateInfoData(BaseItemDataPtr data);

    void UpdateBackTrace(const ListItemVec &data);

    void ClearData();
private slots:
    void TableDoubleClickSlots(QTableWidgetItem *item);
private:
    void InitWidget();
private:
    Ui::DebugWidget *ui;
    class DataPrivate;
    DataPrivate *_p;
};

#endif // DEBUGWIDGET_H
