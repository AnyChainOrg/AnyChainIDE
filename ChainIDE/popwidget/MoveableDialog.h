#ifndef MOVEABLEDIALOG_H
#define MOVEABLEDIALOG_H

#include <QPoint>
#include <QDialog>
#include <QMouseEvent>

class MoveableDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MoveableDialog(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent*event);
    void mouseMoveEvent(QMouseEvent *event);
private:
    QPoint move_point;
};

#endif // MOVEABLEDIALOG_H
