#include "MessageTypeShowWidget.h"
#include "ui_MessageTypeShowWidget.h"

#include <QTextBrowser>
#include <QScrollBar>

MessageTypeShowWidget::MessageTypeShowWidget(QWidget *parent) :
    MoveableWidget(parent),
    ui(new Ui::MessageTypeShowWidget)
{
    ui->setupUi(this);
    InitWidget();
}

MessageTypeShowWidget::~MessageTypeShowWidget()
{
    delete ui;
}

void MessageTypeShowWidget::ReceiveMessage(const QString &message, const QString &messageLabel)
{
    QTextBrowser *bro = nullptr;
    for(int i = 0;i < ui->tabWidget->count();++i)
    {
        if(ui->tabWidget->tabText(i) == messageLabel)
        {
            bro = dynamic_cast<QTextBrowser*>(ui->tabWidget->widget(i));
            break;
        }
    }
    if(bro == nullptr)
    {
        bro = new QTextBrowser(ui->tabWidget);
        ui->tabWidget->addTab(bro,messageLabel);
    }
    bro->append(message);
    QScrollBar *scrollbar = bro->verticalScrollBar();
    if(scrollbar)
    {
        scrollbar->setSliderPosition(scrollbar->maximum());
    }
}

void MessageTypeShowWidget::InitWidget()
{
    setWindowFlags(Qt::FramelessWindowHint | windowFlags() | Qt::WindowStaysOnTopHint);
    connect(ui->closeBtn,&QToolButton::clicked,this,&MessageTypeShowWidget::close);
}
