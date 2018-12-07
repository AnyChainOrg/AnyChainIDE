#include "TransferToContractCTC.h"
#include "ui_TransferToContractCTC.h"

#include <math.h>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "ChainIDE.h"
#include "datamanager/DataManagerCTC.h"
#include "IDEUtil.h"

#include "ConvenientOp.h"

Q_DECLARE_METATYPE(DataManagerStruct::AccountCTC::AccountInfoPtr)
Q_DECLARE_METATYPE(DataManagerStruct::AccountCTC::AssetInfoPtr)

TransferToContractCTC::TransferToContractCTC(QWidget *parent) :
    MoveableDialog(parent),
    ui(new Ui::TransferToContractCTC)
{
    ui->setupUi(this);
    InitWidget();
}

TransferToContractCTC::~TransferToContractCTC()
{
    delete ui;
}

void TransferToContractCTC::transferSlot()
{
    if(ui->comboBox_account->currentText().isEmpty() || ui->comboBox_asset->currentText().isEmpty() ||
       ui->lineEdit_address->text().isEmpty() || ui->doubleSpinBox_ammount->value() < 1e-8)
    {
        return;
    }
    ChainIDE::getInstance()->postRPC("transtocontract_formal",IDEUtil::toJsonFormat("wallet_transfer_to_contract",QJsonArray()<<
                                      ui->doubleSpinBox_ammount->text()<<ui->comboBox_asset->currentText()<<ui->comboBox_account->currentText()
                                      << ui->lineEdit_address->text()<<ui->fee->text()));
}

void TransferToContractCTC::testTransfer()
{
    if(ui->comboBox_account->currentText().isEmpty() || ui->comboBox_asset->currentText().isEmpty() ||
       ui->lineEdit_address->text().isEmpty() || ui->doubleSpinBox_ammount->value() < 1e-8)
    {
        return;
    }
    ChainIDE::getInstance()->postRPC("transtocontract_test",IDEUtil::toJsonFormat("wallet_transfer_to_contract_testing",QJsonArray()<<
                                      ui->doubleSpinBox_ammount->text()<<ui->comboBox_asset->currentText()<<ui->comboBox_account->currentText()
                                      << ui->lineEdit_address->text()));

}

void TransferToContractCTC::comboBoxAccountChangeSlot()
{
    ui->comboBox_asset->clear();
    DataManagerStruct::AccountCTC::AccountInfoPtr info = ui->comboBox_account->currentData().value<DataManagerStruct::AccountCTC::AccountInfoPtr>();
    for(auto it = info->getAssetInfos().begin();it != info->getAssetInfos().end();++it){
        ui->comboBox_asset->addItem((*it)->GetAssetType(),QVariant::fromValue<DataManagerStruct::AccountCTC::AssetInfoPtr>(*it));
    }
    if(ui->comboBox_asset->count() >= 1)
    {
        ui->comboBox_asset->setCurrentIndex(0);
    }

}

void TransferToContractCTC::comboBoxAssetChangeSlot()
{
    //设置上限
    if(DataManagerStruct::AccountCTC::AssetInfoPtr asset = ui->comboBox_asset->currentData().value<DataManagerStruct::AccountCTC::AssetInfoPtr>()){
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

void TransferToContractCTC::jsonDataUpdated(const QString &id, const QString &data)
{
    if("transtocontract_formal" == id)
    {
        ConvenientOp::ShowSyncCommonDialog(data);
        close();
    }
    else if("transtocontract_test" == id)
    {
        ui->fee->setValue(parseTestCallFee(data));
        ui->fee->setToolTip(tr("approximatefee:%1").arg(ui->fee->text()));
    }
}

void TransferToContractCTC::InitWidget()
{
    ui->fee->setRange(0,std::numeric_limits<double>::max());
    ui->fee->setDecimals(8);
    //
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    ui->doubleSpinBox_ammount->setDecimals(8);
    ui->doubleSpinBox_ammount->setSingleStep(0.001);
    ui->doubleSpinBox_ammount->setMaximum(1e18);

    connect(DataManagerCTC::getInstance(),&DataManagerCTC::queryAccountFinish,this,&TransferToContractCTC::InitComboBox);

    connect(ui->closeBtn,&QToolButton::clicked,this,&QWidget::close);
    connect(ui->cancelBtn,&QToolButton::clicked,this,&QWidget::close);
    connect(ui->okBtn,&QToolButton::clicked,this,&TransferToContractCTC::transferSlot);
    connect(ChainIDE::getInstance(),&ChainIDE::jsonDataUpdated,this,&TransferToContractCTC::jsonDataUpdated);
    connect(ui->comboBox_account,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&TransferToContractCTC::comboBoxAccountChangeSlot);
    connect(ui->comboBox_asset,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&TransferToContractCTC::comboBoxAssetChangeSlot);


    connect(ui->comboBox_account,static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),this,
            &TransferToContractCTC::testTransfer);
    connect(ui->comboBox_asset,static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),this,
            &TransferToContractCTC::testTransfer);
    connect(ui->doubleSpinBox_ammount,static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),this,
            &TransferToContractCTC::testTransfer);
    connect(ui->lineEdit_address,&QLineEdit::editingFinished,this,&TransferToContractCTC::testTransfer);

    //初始化
    DataManagerCTC::getInstance()->queryAccount();
}

void TransferToContractCTC::InitComboBox()
{
    DataManagerStruct::AccountCTC::AccountDataPtr accounts = DataManagerCTC::getInstance()->getAccount();
    for(auto it = accounts->getAccount().begin();it != accounts->getAccount().end();++it)
    {
        ui->comboBox_account->addItem((*it)->getAccountName(),QVariant::fromValue<DataManagerStruct::AccountCTC::AccountInfoPtr>(*it));
    }

    if(ui->comboBox_account->count() >= 1)
    {
        ui->comboBox_account->setCurrentIndex(0);
    }
}

double TransferToContractCTC::parseTestCallFee(const QString &data) const
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
        if(addr.toObject().value("amount").isString())
        {
            resultVal += addr.toObject().value("amount").toString().toULongLong()/pow(10,8);
        }
        else if(addr.toObject().value("amount").isDouble())
        {
            resultVal += addr.toObject().value("amount").toDouble()/pow(10,8);
        }

    }
    return resultVal;
}
