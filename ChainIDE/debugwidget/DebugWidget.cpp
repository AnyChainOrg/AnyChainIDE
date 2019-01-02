#include "DebugWidget.h"
#include "ui_DebugWidget.h"

#include <QDebug>
#include "DataTreeItemModel.h"
#include "DebugDataStruct.h"

Q_DECLARE_METATYPE(ListItemDataPtr)

class DebugWidget::DataPrivate
{
public:
    DataPrivate()
        :treeModel(new DataTreeItemModel())
    {

    }
    DataTreeItemModel *treeModel ;
};

DebugWidget::DebugWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugWidget),
    _p(new DataPrivate())
{
    ui->setupUi(this);
    InitWidget();
}

DebugWidget::~DebugWidget()
{
    delete _p;
    _p = nullptr;
    delete ui;
}

void DebugWidget::UpdateInfoData(BaseItemDataPtr data)
{
    _p->treeModel->InitModelData(data);
    ui->treeView->expandAll();
}

void DebugWidget::UpdateBackTrace(const ListItemVec & data)
{
    ui->tableWidget->setRowCount(0);
    std::for_each(data.begin(),data.end(),[this](ListItemDataPtr info){
        int rowCount = this->ui->tableWidget->rowCount();
        this->ui->tableWidget->insertRow(rowCount);

        QTableWidgetItem *itemLevel = new QTableWidgetItem(QString::number(info->GetLevel()));
        itemLevel->setToolTip(itemLevel->text());
        itemLevel->setTextAlignment(Qt::AlignCenter);
        itemLevel->setData(Qt::UserRole,QVariant::fromValue<ListItemDataPtr>(info));
        this->ui->tableWidget->setItem(rowCount,0,itemLevel);

        QTableWidgetItem *itemFunction = new QTableWidgetItem(info->GetFunction());
        itemFunction->setToolTip(itemFunction->text());
        itemFunction->setTextAlignment(Qt::AlignCenter);
        itemFunction->setData(Qt::UserRole,QVariant::fromValue<ListItemDataPtr>(info));
        this->ui->tableWidget->setItem(rowCount,1,itemFunction);

        QTableWidgetItem *itemFile = new QTableWidgetItem(info->GetFile());
        itemFile->setToolTip(itemFile->text());
        itemFile->setTextAlignment(Qt::AlignCenter);
        itemFile->setData(Qt::UserRole,QVariant::fromValue<ListItemDataPtr>(info));
        this->ui->tableWidget->setItem(rowCount,2,itemFile);

        QTableWidgetItem *itemLine = new QTableWidgetItem(QString::number(info->GetLine()));
        itemLine->setToolTip(itemLine->text());
        itemLine->setTextAlignment(Qt::AlignCenter);
        itemLine->setData(Qt::UserRole,QVariant::fromValue<ListItemDataPtr>(info));
        this->ui->tableWidget->setItem(rowCount,3,itemLine);
    });
}

void DebugWidget::ClearData()
{
    _p->treeModel->InitModelData(std::make_shared<BaseItemData>());
    ui->tableWidget->setRowCount(0);
}

void DebugWidget::TableDoubleClickSlots(QTableWidgetItem *item)
{
    if(item && item->data(Qt::UserRole).value<ListItemDataPtr>())
    {
        int line =  item->data(Qt::UserRole).value<ListItemDataPtr>()->GetLine();
        if(line > 0)
        {
            emit JumpToLine(line - 1);
        }
    }
}

void DebugWidget::InitWidget()
{
    ui->splitter->setSizes(QList<int>()<<static_cast<int>(0.67*this->height())<<static_cast<int>(0.33*this->height()));
    ui->treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->treeView->setModel(_p->treeModel);
    ui->tableWidget->horizontalHeader()->setHighlightSections(false);

    connect(ui->tableWidget,&QTableWidget::itemDoubleClicked,this,&DebugWidget::TableDoubleClickSlots);
}
