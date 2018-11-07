#include "TransferWidgetUB.h"
#include "ui_TransferWidgetUB.h"

#include <limits>
#include "ChainIDE.h"
#include "datamanager/DataManagerUB.h"
#include "IDEUtil.h"

#include "ConvenientOp.h"

TransferWidgetUB::TransferWidgetUB(QWidget *parent) :
    MoveableDialog(parent),
    ui(new Ui::TransferWidgetUB)
{
    ui->setupUi(this);
    InitWidget();
}

TransferWidgetUB::~TransferWidgetUB()
{
    delete ui;
}

void TransferWidgetUB::transferSlot()
{
    if(!ui->checkBox->isChecked())
    {
        ChainIDE::getInstance()->postRPC("transfer-to",IDEUtil::toJsonFormat("sendtoaddress",QJsonArray()<<
                                         ui->lineEdit->text()<<ui->doubleSpinBox->value()));
    }
    else
    {
        ChainIDE::getInstance()->postRPC("transfer-to",IDEUtil::toJsonFormat("sendfrom",QJsonArray()<<
                                         ui->comboBox->currentText()<<ui->lineEdit->text()<<ui->doubleSpinBox->value()));
    }

    if(ChainIDE::getInstance()->getCurrentChainType() == DataDefine::TEST)
    {
        //产一个块来确认
        ChainIDE::getInstance()->postRPC("generate",IDEUtil::toJsonFormat("generate",QJsonArray()<<1));
    }
}

void TransferWidgetUB::checkBoxChangeSlot()
{
    ui->comboBox->setEnabled(ui->checkBox->isChecked());
    ui->label_balance->setEnabled(ui->checkBox->isChecked());
    if(!ui->checkBox->isChecked())
    {
        ui->doubleSpinBox->setRange(0,std::numeric_limits<double>::max());
    }
}

void TransferWidgetUB::comboBoxChangeSlot()
{
    ui->label_balance->setText(QString::number(ui->comboBox->currentData().toDouble(),'f',8));
    ui->doubleSpinBox->setRange(0,ui->comboBox->currentData().toDouble());
}

void TransferWidgetUB::addressChangeSlot()
{
    DataManagerUB::getInstance()->checkAddress(ui->lineEdit->text());
}

void TransferWidgetUB::addressCheckSlot(bool isvalid)
{
    QString enable = DataDefine::Black_Theme == ChainIDE::getInstance()->getCurrentTheme() ? "QLineEdit{color:white;}":"QLineEdit{color:black;}";
    QString disable = "QLineEdit{color:red;}";
    ui->lineEdit->setStyleSheet(isvalid?enable:disable);
}

void TransferWidgetUB::jsonDataUpdated(const QString &id, const QString &data)
{
    if("transfer-to" == id)
    {
        ConvenientOp::ShowSyncCommonDialog(data);
        close();
    }
}

void TransferWidgetUB::InitWidget()
{
    //
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    ui->checkBox->setChecked(true);
    ui->doubleSpinBox->setDecimals(8);
    ui->doubleSpinBox->setSingleStep(0.001);
    ui->doubleSpinBox->setRange(0,std::numeric_limits<double>::max());

    connect(DataManagerUB::getInstance(),&DataManagerUB::addressCheckFinish,this,&TransferWidgetUB::addressCheckSlot);
    connect(DataManagerUB::getInstance(),&DataManagerUB::queryAccountFinish,this,&TransferWidgetUB::InitComboBox);
    DataManagerUB::getInstance()->queryAccount();

    connect(ui->closeBtn,&QToolButton::clicked,this,&QWidget::close);
    connect(ui->cancelBtn,&QToolButton::clicked,this,&QWidget::close);
    connect(ui->okBtn,&QToolButton::clicked,this,&TransferWidgetUB::transferSlot);
    connect(ui->checkBox,&QCheckBox::clicked,this,&TransferWidgetUB::checkBoxChangeSlot);
    connect(ui->lineEdit,&QLineEdit::textEdited,this,&TransferWidgetUB::addressChangeSlot);
    connect(ChainIDE::getInstance(),&ChainIDE::jsonDataUpdated,this,&TransferWidgetUB::jsonDataUpdated);

}

void TransferWidgetUB::InitComboBox()
{
    DataManagerStruct::AccountUB::AccountDataPtr accounts = DataManagerUB::getInstance()->getAccount();
    for(auto it = accounts->getAccount().begin();it != accounts->getAccount().end();++it)
    {
        ui->comboBox->addItem((*it)->getAccountName(),(*it)->getTotalBalance());
    }
    connect(ui->comboBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&TransferWidgetUB::comboBoxChangeSlot);

    if(ui->comboBox->count() >= 1)
    {
        ui->comboBox->setCurrentIndex(0);
        comboBoxChangeSlot();
    }
}
