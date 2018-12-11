#include "waitingforsync.h"
#include "ui_waitingforsync.h"

#include <QScrollBar>

WaitingForSync::WaitingForSync(QWidget *parent) :
    MoveableDialog(parent),
    ui(new Ui::WaitingForSync)
{
    ui->setupUi(this);
    InitWidget();
}

WaitingForSync::~WaitingForSync()
{
    delete ui;
}

void WaitingForSync::ReceiveMessage(const QString &message,int messageType)
{
    if(message.isEmpty()) return;
    ui->waitSync->append(message);
    QScrollBar *scrollbar = ui->waitSync->verticalScrollBar();
    if(scrollbar)
    {
        scrollbar->setSliderPosition(scrollbar->maximum());
    }
}

void WaitingForSync::InitWidget()
{
    setWindowFlags(  this->windowFlags() | Qt::FramelessWindowHint);
    ui->title_Label->setVisible(false);
    ui->loadingLabel->setVisible(false);
    ui->waitSync->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

