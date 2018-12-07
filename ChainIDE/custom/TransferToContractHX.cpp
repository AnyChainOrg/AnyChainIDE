#include "TransferToContractHX.h"
#include "ui_TransferToContractHX.h"

#include <math.h>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "ChainIDE.h"
#include "datamanager/DataManagerHX.h"
#include "IDEUtil.h"

#include "ConvenientOp.h"

Q_DECLARE_METATYPE(DataManagerStruct::AccountHX::AccountInfoPtr)
Q_DECLARE_METATYPE(DataManagerStruct::AccountHX::AssetInfoPtr)

TransferToContractHX::TransferToContractHX(QWidget *parent) :
    MoveableDialog(parent),
    ui(new Ui::TransferToContractHX)
{
    ui->setupUi(this);
    InitWidget();
}

TransferToContractHX::~TransferToContractHX()
{
    delete ui;
}

void TransferToContractHX::transferSlot()
{
    ChainIDE::getInstance()->postRPC("transtocontract_formal",IDEUtil::toJsonFormat("transfer_to_contract",QJsonArray()<<
                                      ui->comboBox_account->currentText()<< ui->lineEdit_address->text()<<ui->doubleSpinBox_ammount->text()
                                      <<ui->comboBox_asset->currentText()<<""<<ui->gasprice->text()<<ui->gaslimit->text()<<true));
}

void TransferToContractHX::testTransfer()
{
    if(ui->comboBox_account->currentText().isEmpty() || ui->comboBox_asset->currentText().isEmpty() ||
       ui->lineEdit_address->text().isEmpty() || ui->doubleSpinBox_ammount->value() < 1e-8)
    {
        return;
    }
    ChainIDE::getInstance()->postRPC("transtocontract_test",IDEUtil::toJsonFormat("transfer_to_contract_testing",QJsonArray()<<
                                      ui->comboBox_account->currentText()<< ui->lineEdit_address->text()<<ui->doubleSpinBox_ammount->text()
                                      <<ui->comboBox_asset->currentText()<<""));

}

void TransferToContractHX::comboBoxAccountChangeSlot()
{
    ui->comboBox_asset->clear();
    DataManagerStruct::AccountHX::AccountInfoPtr info = ui->comboBox_account->currentData().value<DataManagerStruct::AccountHX::AccountInfoPtr>();
    for(auto it = info->getAssetInfos().begin();it != info->getAssetInfos().end();++it){
        ui->comboBox_asset->addItem((*it)->GetAssetType(),QVariant::fromValue<DataManagerStruct::AccountHX::AssetInfoPtr>(*it));
    }
    if(ui->comboBox_asset->count() >= 1)
    {
        ui->comboBox_asset->setCurrentIndex(0);
    }

}

void TransferToContractHX::comboBoxAssetChangeSlot()
{
    //设置上限
    if(DataManagerStruct::AccountHX::AssetInfoPtr asset = ui->comboBox_asset->currentData().value<DataManagerStruct::AccountHX::AssetInfoPtr>()){
        double number = asset->GetBalance()/pow(10,asset->GetPrecision());
        ui->label_balance->setText(QString::number(number,'f',asset->GetPrecision()));
        ui->doubleSpinBox_ammount->setDecimals(asset->GetPrecision());
        ui->doubleSpinBox_ammount->setRange(0,number);
    }
    else{
        ui->label_balance->setText("/");
        ui->doubleSpinBox_ammount->setRange(0,0);
    }
}

void TransferToContractHX::jsonDataUpdated(const QString &id, const QString &data)
{
    if("transtocontract_formal" == id)
    {
        ConvenientOp::ShowSyncCommonDialog(data);
        close();
    }
    else if("transtocontract_test" == id)
    {
        double testFee = parseTestCallFee(data);
        ui->gaslimit->setToolTip(tr("approximatefee:%1").arg(QString::number(testFee,'f',5)));
        ui->gasprice->setToolTip(tr("approximatefee:%1").arg(QString::number(testFee,'f',5)));
    }
}

void TransferToContractHX::InitWidget()
{

    ui->gaslimit->setRange(0,std::numeric_limits<int>::max());
    ui->gaslimit->setSingleStep(1);
    ui->gasprice->setRange(10,std::numeric_limits<int>::max());
    //
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    ui->doubleSpinBox_ammount->setDecimals(8);
    ui->doubleSpinBox_ammount->setSingleStep(0.001);
    ui->doubleSpinBox_ammount->setMaximum(1e18);

    connect(DataManagerHX::getInstance(),&DataManagerHX::queryAccountFinish,this,&TransferToContractHX::InitComboBox);

    connect(ui->closeBtn,&QToolButton::clicked,this,&QWidget::close);
    connect(ui->cancelBtn,&QToolButton::clicked,this,&QWidget::close);
    connect(ui->okBtn,&QToolButton::clicked,this,&TransferToContractHX::transferSlot);
    connect(ChainIDE::getInstance(),&ChainIDE::jsonDataUpdated,this,&TransferToContractHX::jsonDataUpdated);
    connect(ui->comboBox_account,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&TransferToContractHX::comboBoxAccountChangeSlot);
    connect(ui->comboBox_asset,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&TransferToContractHX::comboBoxAssetChangeSlot);


    connect(ui->comboBox_account,static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),this,
            &TransferToContractHX::testTransfer);
    connect(ui->comboBox_asset,static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),this,
            &TransferToContractHX::testTransfer);
    connect(ui->doubleSpinBox_ammount,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,
            &TransferToContractHX::testTransfer);
    connect(ui->lineEdit_address,&QLineEdit::editingFinished,this,&TransferToContractHX::testTransfer);

    //初始化
    DataManagerHX::getInstance()->queryAccount();

}

void TransferToContractHX::InitComboBox()
{
    DataManagerStruct::AccountHX::AccountDataPtr accounts = DataManagerHX::getInstance()->getAccount();
    for(auto it = accounts->getAccount().begin();it != accounts->getAccount().end();++it)
    {
        ui->comboBox_account->addItem((*it)->getAccountName(),QVariant::fromValue<DataManagerStruct::AccountHX::AccountInfoPtr>(*it));
    }

    if(ui->comboBox_account->count() >= 1)
    {
        ui->comboBox_account->setCurrentIndex(0);
    }
}

double TransferToContractHX::parseTestCallFee(const QString &data) const
{
    double resultVal = 0;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(data.toUtf8(),&json_error);
    if(json_error.error != QJsonParseError::NoError || !parse_doucment.isObject())
    {
        qDebug()<<json_error.errorString();
        return 0;
    }
    QJsonArray resultArray = parse_doucment.object().value("result").toArray();
    foreach (QJsonValue addr, resultArray) {
        if(addr.isObject())
        {
            if(addr.toObject().value("amount").isString())
            {
                resultVal += addr.toObject().value("amount").toString().toULongLong()/pow(10,5);
            }
            else if(addr.toObject().value("amount").isDouble())
            {
                resultVal += addr.toObject().value("amount").toDouble()/pow(10,5);
            }
        }
        else if(addr.isDouble())
        {
            ui->gaslimit->setValue(addr.toInt());
        }
    }
    return resultVal;
}
