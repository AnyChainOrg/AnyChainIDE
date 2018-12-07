#ifndef TRANSFERTOCONTRACTCTC_H
#define TRANSFERTOCONTRACTCTC_H

#include "popwidget/MoveableDialog.h"

namespace Ui {
class TransferToContractCTC;
}

class TransferToContractCTC : public MoveableDialog
{
    Q_OBJECT

public:
    explicit TransferToContractCTC(QWidget *parent = nullptr);
    ~TransferToContractCTC();
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
    Ui::TransferToContractCTC *ui;
};

#endif // TRANSFERTOCONTRACTCTC_H
