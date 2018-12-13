#ifndef FunctionWidget_H
#define FunctionWidget_H

#include <QPoint>
#include <QWidget>

namespace Ui {
class FunctionWidget;
}

class FunctionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FunctionWidget(QWidget *parent = nullptr);
    ~FunctionWidget();
signals:

public:
    void RefreshContractAddr(const QString &addr);
    void Clear();
public slots:
    void retranslator();
private slots:
    void CopyFunction();
    void customContextMenuRequestedSlot(const QPoint &pos);
private:
    void InitWidget();
    void InitContextMenu();
private:
    Ui::FunctionWidget *ui;
    class DataPrivate;
    DataPrivate *_p;
};

#endif // FunctionWidget_H
