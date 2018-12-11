#include "MoveableWidget.h"

MoveableWidget::MoveableWidget(QWidget *parent) : QWidget(parent)
{

}

void MoveableWidget::mousePressEvent(QMouseEvent *event)
{

    if(event->buttons() & Qt::LeftButton)
    {
         //鼠标相对于窗体的位置（或者使用event->globalPos() - this->pos()）
         move_point = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void MoveableWidget::mouseMoveEvent(QMouseEvent *event)
{
    //若鼠标左键被按下
    if(event->buttons()& Qt::LeftButton)
    {
        //移动主窗体位置
        this->move(event->globalPos() - move_point);
    }
    QWidget::mouseMoveEvent(event);
}
