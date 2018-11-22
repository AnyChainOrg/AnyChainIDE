#ifndef ABOUTWIDGETGIT_H
#define ABOUTWIDGETGIT_H

#include "MoveableDialog.h"

namespace Ui {
class AboutWidgetGit;
}

class AboutWidgetGit : public MoveableDialog
{
    Q_OBJECT

public:
    explicit AboutWidgetGit(QWidget *parent = nullptr);
    ~AboutWidgetGit();
private slots:
    void checkNewVersion();
private:
    void InitWidget();
private:
    Ui::AboutWidgetGit *ui;
    class DataPrivate;
    DataPrivate *_p;
};

#endif // ABOUTWIDGETGIT_H
