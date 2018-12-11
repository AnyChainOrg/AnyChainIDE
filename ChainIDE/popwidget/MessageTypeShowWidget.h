#ifndef MESSAGETYPESHOWWIDGET_H
#define MESSAGETYPESHOWWIDGET_H

#include "popwidget/MoveableWidget.h"

namespace Ui {
class MessageTypeShowWidget;
}

class MessageTypeShowWidget : public MoveableWidget
{
    Q_OBJECT

public:
    explicit MessageTypeShowWidget(QWidget *parent = nullptr);
    ~MessageTypeShowWidget();
public slots:
    void ReceiveMessage(const QString &message,const QString &messageLabel);
private:
    void InitWidget();
private:
    Ui::MessageTypeShowWidget *ui;
};

#endif // MESSAGETYPESHOWWIDGET_H
