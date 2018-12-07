#ifndef TRANSFERTOCONTRACTHX_H
#define TRANSFERTOCONTRACTHX_H

#include "popwidget/MoveableDialog.h"

namespace Ui {
class TransferToContractHX;
}

class TransferToContractHX : public MoveableDialog
{
    Q_OBJECT

public:
    explicit TransferToContractHX(QWidget *parent = nullptr);
    ~TransferToContractHX();
private slots:
    void transferSlot();
    void testTransfer();//测试转账费用
    void comboBoxAccountChangeSlot();
    void comboBoxAssetChangeSlot();
    void jsonDataUpdated(const QString &id,const QString &data);
private:
    void InitWidget();
    void InitComboBox();
private:
    double parseTestCallFee(const QString &data) const;
private:
    Ui::TransferToContractHX *ui;
};

#endif // TRANSFERTOCONTRACTHX_H
