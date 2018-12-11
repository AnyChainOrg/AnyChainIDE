#ifndef MOVEABLEWIDGET_H
#define MOVEABLEWIDGET_H

#include <QPoint>
#include <QWidget>
#include <QMouseEvent>

class MoveableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MoveableWidget(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent*event);
    void mouseMoveEvent(QMouseEvent *event);
private:
    QPoint move_point;
};

#endif // MOVEABLEWIDGET_H
