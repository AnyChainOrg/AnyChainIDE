#include "InterfaceWidget.h"
#include "ui_InterfaceWidget.h"

#include <QFileInfo>
#include <QDir>
#include <QTextCodec>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QToolButton>
#include <QDebug>
#include <QTabBar>
#include <QMenu>
#include <QClipboard>

#include "IDEUtil.h"
#include "ConvenientOp.h"

class InterfaceWidget::DataPrivate
{
public:
    DataPrivate()
        :data(std::make_shared<DataDefine::ApiEvent>())
        ,contextMenu(new QMenu())
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
    QString currentFilePath;
    DataDefine::ApiEventPtr data;
    QMenu *contextMenu;//右键菜单
};

InterfaceWidget::InterfaceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InterfaceWidget),
    _p(new DataPrivate())
{
    ui->setupUi(this);
    InitWidget();
}

InterfaceWidget::~InterfaceWidget()
{
    delete _p;
    delete ui;
}

void InterfaceWidget::ShowInterface(const QString &filePath)
{
    _p->currentFilePath = filePath;
    InitData();
}

void InterfaceWidget::retranslator()
{
    ui->retranslateUi(this);
}

void InterfaceWidget::InitData()
{
    ui->treeWidget_event->clear();
    ui->treeWidget_function->clear();
    if(_p->currentFilePath.isEmpty()) return;
    //寻找对应的.meta.json，，glua文件寻找自身名字的.meta.json文件，，java文件寻找自身所在目录的.meta.json文件
    QFileInfo file(_p->currentFilePath);
    QString filePath;
    if(_p->currentFilePath.endsWith("."+DataDefine::GLUA_SUFFIX)||
       _p->currentFilePath.endsWith("."+DataDefine::JAVA_SUFFIX)||
       _p->currentFilePath.endsWith("."+DataDefine::CSHARP_SUFFIX)||
       _p->currentFilePath.endsWith("."+DataDefine::KOTLIN_SUFFIX)
       )
    {
        filePath = ConvenientOp::GetMetaJsonFile(_p->currentFilePath);
    }
    else if(_p->currentFilePath.endsWith("."+DataDefine::CONTRACT_SUFFIX))
    {
        filePath = file.absoluteDir().absolutePath()+QDir::separator()+file.dir().dirName() + "."+DataDefine::META_SUFFIX;
    }

    if(ConvenientOp::readApiFromPath(filePath,_p->data))
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList()<<"api");
        item->setFlags(Qt::ItemIsEnabled);
        ui->treeWidget_function->addTopLevelItem(item);

        QTreeWidgetItem *item1 = new QTreeWidgetItem(QStringList()<<"offline-api");
        item1->setFlags(Qt::ItemIsEnabled);
        ui->treeWidget_function->addTopLevelItem(item1);

        QStringList apis = _p->data->getAllApiName();
        foreach (QString api, apis) {
            QTreeWidgetItem *itema = new QTreeWidgetItem(QStringList()<<api);
            item->addChild(itema);
        }

        QStringList off_apis = _p->data->getAllOfflineApiName();
        foreach (QString api, off_apis) {
            QTreeWidgetItem *itema = new QTreeWidgetItem(QStringList()<<api);
            item1->addChild(itema);
        }
        ui->treeWidget_function->expandAll();

        QStringList events = _p->data->getAllEventName();
        foreach (QString api, events) {
            QTreeWidgetItem *item = new QTreeWidgetItem(QStringList()<<api);
            ui->treeWidget_event->addTopLevelItem(item);
        }

    }
}

void InterfaceWidget::InitWidget()
{
    //隐藏左右控制按钮
    ui->tabWidget->tabBar()->setUsesScrollButtons(false);

    ui->treeWidget_event->header()->setVisible(false);
    ui->treeWidget_function->header()->setVisible(false);
    ui->title_Label->setVisible(false);
    ui->tabWidget->setCurrentIndex(0);

    InitContextMenu();
}

void InterfaceWidget::InitContextMenu()
{
    QAction *copyAction = new QAction(tr("复制"),this);
    connect(copyAction,&QAction::triggered,this,&InterfaceWidget::CopyFunction);
    _p->contextMenu->addAction(copyAction);

    ui->treeWidget_function->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget_function,&QTreeWidget::customContextMenuRequested,this,&InterfaceWidget::customContextMenuRequestedSlot);
    ui->treeWidget_event->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget_event,&QTreeWidget::customContextMenuRequested,this,&InterfaceWidget::customContextMenuRequestedSlot);
}

void InterfaceWidget::CopyFunction()
{
    QTreeWidget*tree = 0==ui->tabWidget->currentIndex()?ui->treeWidget_function:ui->treeWidget_event;
    if(!tree) return;
    if(QTreeWidgetItem *item = tree->currentItem())
    {
        QApplication::clipboard()->setText(item->text(0));
    }
}

void InterfaceWidget::customContextMenuRequestedSlot(const QPoint &pos)
{
    QTreeWidget *tree = dynamic_cast<QTreeWidget*>(sender());
    if(tree && tree->currentItem())
    {
        _p->contextMenu->exec(QCursor::pos());
    }
}
