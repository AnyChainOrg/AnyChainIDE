#ifndef TransferWidgetUB_H
#define TransferWidgetUB_H

#include "popwidget/MoveableDialog.h"

namespace Ui {
class TransferWidgetUB;
}

class TransferWidgetUB : public MoveableDialog
{
    Q_OBJECT

public:
    explicit TransferWidgetUB(QWidget *parent = nullptr);
    ~TransferWidgetUB();
private slots:
    void transferSlot();
    void checkBoxChangeSlot();
    void comboBoxChangeSlot();
    void addressChangeSlot();
    void addressCheckSlot(bool isvalid);
    void jsonDataUpdated(const QString &id,const QString &data);
private:
    void InitWidget();
    void InitComboBox();
private:
    Ui::TransferWidgetUB *ui;
};

#endif // TRANSFERWIDGET_H
