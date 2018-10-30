#include "tabcontextmenu.h"

TabContextMenu::TabContextMenu(MenuTypes types,QWidget *parent)
    : QMenu(parent)
    ,type(types)
{
    InitWidget();
}

TabContextMenu::~TabContextMenu()
{

}

void TabContextMenu::InitWidget()
{
    if(type & TabContextMenu::CloseOther)
    {
        QAction* newAction = new QAction(tr("Close Other"),this);
        connect(newAction,&QAction::triggered,this,&TabContextMenu::closeOther);
        addAction(newAction);

    }
    if(type & TabContextMenu::CloseAll)
    {
        QAction* newAction = new QAction(tr("Close All"),this);
        connect(newAction,&QAction::triggered,this,&TabContextMenu::closeAll);
        this->addAction(newAction);

    }
    if(type & TabContextMenu::ViewInExplorer)
    {
        this->addSeparator();
        QAction* newAction = new QAction(tr("Copy File Path"),this);
        connect(newAction,&QAction::triggered,this,&TabContextMenu::copyFilePath);
        this->addAction(newAction);

    }

    if((type & TabContextMenu::ViewInExplorer) | (type & TabContextMenu::OpenWithNative))
    {
        this->addSeparator();
    }

    if(type & TabContextMenu::ViewInExplorer)
    {
        QAction* newAction = new QAction(tr("View In Explorer"),this);
        connect(newAction,&QAction::triggered,this,&TabContextMenu::viewInExplorer);
        this->addAction(newAction);

    }
    if(type & TabContextMenu::OpenWithNative)
    {
        QAction* newAction = new QAction(tr("Open With Native App"),this);
        connect(newAction,&QAction::triggered,this,&TabContextMenu::openWithNative);
        this->addAction(newAction);
    }
}
