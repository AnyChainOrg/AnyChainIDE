#ifndef TABCONTEXTMENU_H
#define TABCONTEXTMENU_H

#include <QMenu>

class TabContextMenu : public QMenu
{
    Q_OBJECT
public:
    enum MenuType{ CloseOther = 1 << 0,
                   CloseAll = 1 << 1,
                   ViewInExplorer = 1 << 2,
                   OpenWithNative = 1 << 3,
                   CopyPath = 1 << 4
                 };
    typedef int MenuTypes;

    explicit TabContextMenu(MenuTypes types,QWidget *parent = nullptr);
    ~TabContextMenu();

signals:
    void closeAll();
    void closeOther();
    void viewInExplorer();
    void openWithNative();
    void copyFilePath();
private:
    void InitWidget();
private:
    MenuTypes type;
};

#endif // TABCONTEXTMENU_H
