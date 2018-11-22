#ifndef DEBUGFUNCTIONWIDGET_H
#define DEBUGFUNCTIONWIDGET_H

#include <QDialog>
#include "DataDefine.h"
#include "popwidget/MoveableDialog.h"

namespace Ui {
class DebugFunctionWidget;
}

class DebugFunctionWidget : public MoveableDialog
{
    Q_OBJECT

public:
    explicit DebugFunctionWidget(const QString &sourceFile,const QString &gpcFile,QWidget *parent = nullptr);
    ~DebugFunctionWidget();
public:
    const QString &SelectedApi()const;
    const QStringList &ApiParams()const;
private slots:
    void OnOKClicked();
    void OnResetClicked();
private:
    void InitWidget();
    void InitFunction();
    void ResetStorageData();
private:
    Ui::DebugFunctionWidget *ui;
    class DataPrivate;
    DataPrivate *_p;
};

#endif // DEBUGFUNCTIONWIDGET_H
