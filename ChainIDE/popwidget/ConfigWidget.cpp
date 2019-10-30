#include "ConfigWidget.h"
#include "ui_ConfigWidget.h"

#include <QPushButton>
#include <QFileDialog>
#include "DataDefine.h"
#include "ChainIDE.h"
#include "ConvenientOp.h"
ConfigWidget::ConfigWidget(QWidget *parent) :
    MoveableDialog(parent),
    ui(new Ui::ConfigWidget)
{
    ui->setupUi(this);
    InitWidget();
}

ConfigWidget::~ConfigWidget()
{
    delete ui;
}

bool ConfigWidget::pop()
{
    exec();
    return isOk;
}

void ConfigWidget::ConfirmSlots()
{
    ChainIDE::getInstance()->setCurrentLanguage(static_cast<DataDefine::Language>(ui->language->currentData().toInt()));
    ChainIDE::getInstance()->setCurrentTheme(static_cast<DataDefine::ThemeStyle>(ui->theme->currentData().toInt()));
    ChainIDE::getInstance()->setChainClass(static_cast<DataDefine::BlockChainClass>(ui->chainclass->currentData().toInt()));
    ChainIDE::getInstance()->setStartChainTypes(static_cast<DataDefine::ChainTypes>(ui->starttype->currentData().toUInt()));
    ChainIDE::getInstance()->setConfigAppDataPath(ui->dataPath->text());
    ChainIDE::getInstance()->setUpdateServer(ui->updateServer->text());

    isOk = true;
    if(ChainIDE::getInstance()->getCurrentTheme() != ui->theme->currentData().toInt() ||
       ChainIDE::getInstance()->getChainClass() != ui->chainclass->currentData().toInt() ||
       ChainIDE::getInstance()->getStartChainTypes() != ui->starttype->currentData().toInt() ||
       ChainIDE::getInstance()->getConfigAppDataPath() != ui->dataPath->text())
    {
        ConvenientOp::ShowSyncCommonDialog(tr("the change will take effect after restart!"));
    }
    close();
}

void ConfigWidget::ChangeDataPath()
{
    QString file = QFileDialog::getExistingDirectory(this,tr( "Select the path to store the blockchain"));
    if( !file.isEmpty())
    {
        ui->dataPath->setText( file);
    }
}

void ConfigWidget::InitWidget()
{
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    isOk = false;
    //初始化language
    ui->language->clear();
    ui->language->addItem(tr("  English"),static_cast<int>(DataDefine::English));
    ui->language->addItem(tr("  Chinese"),static_cast<int>(DataDefine::SimChinese));
    if(ChainIDE::getInstance()->getCurrentLanguage() ==DataDefine::English)
    {
        ui->language->setCurrentIndex(0);
    }
    else if(ChainIDE::getInstance()->getCurrentLanguage() ==DataDefine::SimChinese)
    {
        ui->language->setCurrentIndex(1);
    }

    //初始化主题
    ui->theme->clear();
    ui->theme->addItem(tr("  Black"),static_cast<int>(DataDefine::Black_Theme));
    ui->theme->addItem(tr("  White"),static_cast<int>(DataDefine::White_Theme));
    if(ChainIDE::getInstance()->getCurrentThemeConfig() ==DataDefine::Black_Theme)
    {
        ui->theme->setCurrentIndex(0);
    }
    else if(ChainIDE::getInstance()->getCurrentThemeConfig() ==DataDefine::White_Theme)
    {
        ui->theme->setCurrentIndex(1);
    }

    //初始化链类型
    ui->chainclass->clear();
    ui->chainclass->addItem(tr("  HX"),static_cast<int>(DataDefine::HX));
    ui->chainclass->addItem(tr("  UB"),static_cast<int>(DataDefine::UB));
    ui->chainclass->addItem(tr("  CTC"),static_cast<int>(DataDefine::CTC));
    ui->chainclass->addItem(tr("  XWC"),static_cast<int>(DataDefine::XWC));
    if(ChainIDE::getInstance()->getChainClassConfig() ==DataDefine::HX)
    {
        ui->chainclass->setCurrentIndex(0);
    }
    else if(ChainIDE::getInstance()->getChainClassConfig() ==DataDefine::UB)
    {
        ui->chainclass->setCurrentIndex(1);
    }
    else if(ChainIDE::getInstance()->getChainClassConfig() ==DataDefine::CTC)
    {
        ui->chainclass->setCurrentIndex(2);
    }
    else if(ChainIDE::getInstance()->getChainClassConfig() ==DataDefine::XWC)
    {
        ui->chainclass->setCurrentIndex(3);
    }
    //初始化启动类型
    ui->starttype->clear();
    ui->starttype->addItem(tr("  Test"),static_cast<int>(DataDefine::TEST));
    ui->starttype->addItem(tr("  Formal"),static_cast<int>(DataDefine::FORMAL));
    ui->starttype->addItem(tr("  None"),static_cast<int>(DataDefine::NONE));
    ui->starttype->addItem(tr("  All"),static_cast<int>(DataDefine::TEST|DataDefine::FORMAL));
    if(ChainIDE::getInstance()->getStartChainTypesConfig() ==DataDefine::TEST)
    {
        ui->starttype->setCurrentIndex(0);
    }
    else if(ChainIDE::getInstance()->getStartChainTypesConfig() ==DataDefine::FORMAL)
    {
        ui->starttype->setCurrentIndex(1);
    }
    else if(ChainIDE::getInstance()->getStartChainTypesConfig() ==DataDefine::NONE)
    {
        ui->starttype->setCurrentIndex(2);
    }
    else if((ChainIDE::getInstance()->getStartChainTypesConfig() & DataDefine::TEST) &&(ChainIDE::getInstance()->getStartChainTypesConfig() & DataDefine::FORMAL))
    {
        ui->starttype->setCurrentIndex(3);
    }

    //初始化数据路径
    ui->dataPath->setText(ChainIDE::getInstance()->getConfigAppDataPathConfig());
    QPushButton *pButton = new QPushButton(this);
    pButton->setStyleSheet("QPushButton{border:none;background:transparent;}");
    pButton->setCursor(Qt::PointingHandCursor);
    pButton->setFixedSize(20, 20);
    pButton->setToolTip(tr("Change"));
    pButton->setIconSize(QSize(20,20));
    pButton->setIcon(QIcon(":/pic/configure_enable.png"));

    //防止文本框输入内容位于按钮之下
    QMargins margins = ui->dataPath->textMargins();
    ui->dataPath->setTextMargins(margins.left(), margins.top(),pButton->width()+5, margins.bottom());

    QHBoxLayout *pLayout = new QHBoxLayout();
    pLayout->addStretch();
    pLayout->addWidget(pButton);
    pLayout->setSpacing(0);
    pLayout->setDirection(QBoxLayout::LeftToRight);
    pLayout->setContentsMargins(0, 0, 5, 0);
    ui->dataPath->setLayout(pLayout);
    connect(pButton,&QPushButton::clicked,this,&ConfigWidget::ChangeDataPath);

    //初始化更新器路径
    ui->updateServer->setText(ChainIDE::getInstance()->getUpdateServer());

    connect(ui->cancelBtn,&QToolButton::clicked,this,&ConfigWidget::close);
    connect(ui->closeBtn,&QToolButton::clicked,this,&ConfigWidget::close);
    connect(ui->okBtn,&QToolButton::clicked,this,&ConfigWidget::ConfirmSlots);

    ui->label_server->setVisible(false);
    ui->updateServer->setVisible(false);
}
