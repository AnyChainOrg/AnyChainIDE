#include "FunctionWidget.h"
#include "ui_FunctionWidget.h"

#include <QDebug>
#include <QMenu>
#include <QClipboard>

#include "ChainIDE.h"
#include "datamanager/DataManagerHX.h"
#include "datamanager/DataManagerUB.h"
#include "datamanager/DataManagerCTC.h"

class FunctionWidget::DataPrivate
{
public:
    DataPrivate()
        :contextMenu(new QMenu())
    {

    }
    ~DataPrivate()
    {
        if(contextMenu)
        {
            delete contextMenu;
            contextMenu = nullptr;
        }
    }
public:
    QMenu *contextMenu;//右键菜单
};

FunctionWidget::FunctionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FunctionWidget),
    _p(new DataPrivate())
{
    ui->setupUi(this);
    InitWidget();
}

FunctionWidget::~FunctionWidget()
{
    delete _p;
    _p = nullptr;
    delete ui;
}

void FunctionWidget::RefreshContractAddr(const QString &addr)
{
    ui->treeWidget_offline->clear();
    ui->treeWidget_online->clear();
    ui->tabWidget->setCurrentIndex(0);

    DataManagerStruct::ContractInfoPtr info = nullptr;
    if(ChainIDE::getInstance()->getChainClass() == DataDefine::HX)
    {
        info = DataManagerHX::getInstance()->getContract()->getContractInfo(addr);
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::UB)
    {
        info = DataManagerUB::getInstance()->getContract()->getContractInfo(addr);
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::CTC)
    {
        info = DataManagerCTC::getInstance()->getContract()->getContractInfo(addr);
    }

    if(!info) return;

    foreach(QString api, info->GetInterface()->getAllApiName())
    {
        QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList()<<api);
        ui->treeWidget_online->addTopLevelItem(itemChild);
    }
    foreach(QString offapi, info->GetInterface()->getAllOfflineApiName())
    {
        QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList()<<offapi);
        ui->treeWidget_offline->addTopLevelItem(itemChild);
    }
}

void FunctionWidget::Clear()
{
    ui->treeWidget_offline->clear();
    ui->treeWidget_online->clear();
    ui->tabWidget->setCurrentIndex(0);
}

void FunctionWidget::InitWidget()
{
    ui->treeWidget_offline->header()->setVisible(false);
    ui->treeWidget_online->header()->setVisible(false);
    InitContextMenu();
}

void FunctionWidget::CopyFunction()
{
    QTreeWidget*tree = 0==ui->tabWidget->currentIndex()?ui->treeWidget_online:ui->treeWidget_offline;
    if(!tree) return;
    if(QTreeWidgetItem *item = tree->currentItem())
    {
        QApplication::clipboard()->setText(item->text(0));
    }
}

void FunctionWidget::customContextMenuRequestedSlot(const QPoint &pos)
{
    QTreeWidget *tree = dynamic_cast<QTreeWidget*>(sender());
    if(tree && tree->currentItem())
    {
        _p->contextMenu->exec(QCursor::pos());
    }
}

void FunctionWidget::InitContextMenu()
{
    QAction *copyAction = new QAction(tr("复制"),this);
    connect(copyAction,&QAction::triggered,this,&FunctionWidget::CopyFunction);
    _p->contextMenu->addAction(copyAction);

    ui->treeWidget_offline->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget_offline,&QTreeWidget::customContextMenuRequested,this,&FunctionWidget::customContextMenuRequestedSlot);
    ui->treeWidget_online->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget_online,&QTreeWidget::customContextMenuRequested,this,&FunctionWidget::customContextMenuRequestedSlot);
}

void FunctionWidget::retranslator()
{
    ui->retranslateUi(this);
}
