#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include <QMenu>

class ContextMenu : public QMenu
{       
    Q_OBJECT
public:
    enum MenuType{ Delete = 1 << 0,
                   Copy = 1 << 1,
                   Paste = 1 << 2,
                   Import = 1 << 3,
                   Compile = 1 << 4,
                   NewFile = 1 << 5,
                   Export = 1 << 6,
                   Show = 1 << 7,
                   Open = 1 << 8
                 };
    typedef int MenuTypes;

    ContextMenu( MenuTypes type, QWidget * parent = nullptr);
    ~ContextMenu();

signals:
    void newFileTriggered();
    void copyTriggered();
    void pasteTriggered();
    void deleteTriggered();
    void importTriggered();
    void exportTriggered();
    void compileTriggered();
    void showInExplorer();
    void openWithNative();
private:
    MenuTypes type;

    void createActions();
};

#endif // CONTEXTMENU_H
