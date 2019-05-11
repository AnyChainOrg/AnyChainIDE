#include "mainwindow.h"
#include "ui_MainWindow.h"

#include <functional>
#include <QCoreApplication>
#include <QMessageBox>
#include <QTabBar>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>
#include <QDesktopServices>

#include "DataDefine.h"
#include "ChainIDE.h"
#include "compile/CompileManager.h"
#include "backstage/BackStageManager.h"
#include "debugwidget/DebugManager.h"

#include "selectpathwidget.h"
#include "waitingforsync.h"
#include "outputwidget.h"
#include "filewidget/FileWidget.h"
#include "contentwidget/ContextWidget.h"

#include "popwidget/NewFileDialog.h"
#include "popwidget/consoledialog.h"
#include "popwidget/ConfigWidget.h"
#include "popwidget/commondialog.h"
#include "popwidget/AboutWidget.h"
#include "popwidget/AboutWidgetGit.h"
#include "popwidget/MessageTypeShowWidget.h"

#include "datamanager/DataManagerHX.h"
#include "datamanager/DataManagerUB.h"
#include "datamanager/DataManagerCTC.h"

#include "custom/AccountWidgetHX.h"
#include "custom/RegisterContractDialogHX.h"
#include "custom/UpgradeContractDialogHX.h"
#include "custom/TransferWidgetHX.h"
#include "custom/CallContractWidgetHX.h"
#include "custom/PasswordVerifyWidgetHX.h"

#include "custom/AccountWidgetUB.h"
#include "custom/RegisterContractDialogUB.h"
#include "custom/TransferWidgetUB.h"
#include "custom/CallContractWidgetUB.h"

#include "custom/AccountWidgetCTC.h"
#include "custom/TransferWidgetCTC.h"
#include "custom/RegisterContractDialogCTC.h"
#include "custom/CallContractWidgetCTC.h"
#include "custom/UpgradeContractDialogCTC.h"

#include "custom/TransferToContractHX.h"
#include "custom/TransferToContractCTC.h"

#include "debugwidget/DebugFunctionWidget.h"

#include "ConvenientOp.h"
#include "IDEUtil.h"

#ifndef WIN32//目前没有mac版本的调试器
//#define DEBUG_FUNC_OFF
#endif

class MainWindow::DataPrivate
{
public:
    DataPrivate()
        :updateNeeded(false)
        ,backstageMessageShow(nullptr)
    {

    }
    ~DataPrivate()
    {
        if(backstageMessageShow)
        {
            delete backstageMessageShow;
            backstageMessageShow = nullptr;
        }
    }
public:
    bool updateNeeded;//是否需要更新文件---启动copy
    MessageTypeShowWidget *backstageMessageShow;//后台输出显示窗口
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _p(new DataPrivate())
{
    ui->setupUi(this);
    InitWidget();
}

MainWindow::~MainWindow()
{
    qDebug()<<"delete mainwindow";
#ifdef WIN32
    qInstallMessageHandler(nullptr);
#endif
    delete _p;
    _p = nullptr;
    delete ui;
}

void MainWindow::InitWidget()
{
    //设置界面背景色
    refreshStyle();
    //标题
    refreshTitle();
    //翻译
    refreshTranslator();
//    startWidget();
    hide();
    if( ChainIDE::getInstance()->getConfigAppDataPath().isEmpty() || !QFileInfo(ChainIDE::getInstance()->getConfigAppDataPath()).isDir() || !QFileInfo(ChainIDE::getInstance()->getConfigAppDataPath()).exists())
    {
        showSelectPathWidget();
    }
    else
    {
        startChain();
    }
}

void MainWindow::showSelectPathWidget()
{
    SelectPathWidget* selectPathWidget = new SelectPathWidget();
    selectPathWidget->setWindowTitle( QString::fromLocal8Bit("IDE"));
    selectPathWidget->setAttribute(Qt::WA_DeleteOnClose);
    connect( selectPathWidget,&SelectPathWidget::enter,this,&MainWindow::startChain);
    connect( selectPathWidget,&SelectPathWidget::cancel,this,&MainWindow::close);

    selectPathWidget->show();
}

void MainWindow::startChain()
{
    if(ChainIDE::getInstance()->getStartChainTypes() == DataDefine::NONE)
    {
        //直接开启窗口
        startWidget();
        return;
    }
    //显示等待窗口
    showWaitingForSyncWidget();

    //启动后台
    connect(ChainIDE::getInstance()->getBackStageManager(),&BackStageManager::startBackStageFinish,this,&MainWindow::exeStartedSlots);
    connect(ChainIDE::getInstance()->getBackStageManager(),&BackStageManager::backStageRunError,this,&MainWindow::exeErrorSlots);
    ChainIDE::getInstance()->getBackStageManager()->startBackStage();
}

void MainWindow::startWidget()
{
    //最大化
    showMaximized();

    //隐藏部分不需要的按钮
    HideAction();

    //设置界面比例
    ui->splitter_hori->setSizes(QList<int>()<<static_cast<int>(0.1*ui->centralWidget->width())//文件管理栏
                                            <<static_cast<int>(0.9*ui->centralWidget->width())//中央代码栏
                                            <<static_cast<int>(0.2*ui->centralWidget->width()));//调试窗
    ui->splitter_ver->setSizes(QList<int>()<<static_cast<int>(0.67*ui->centralWidget->height())//代码编辑栏
                                           <<static_cast<int>(0.33*ui->centralWidget->height()));//程序输出栏
    ui->debugWidget->setVisible(false);

    //链接编译槽
    connect(ChainIDE::getInstance()->getCompileManager(),&CompileManager::CompileOutput,ui->outputWidget,&OutputWidget::receiveCompileMessage);
    connect(ChainIDE::getInstance()->getCompileManager(),&CompileManager::finishCompile,std::bind(&QAction::setEnabled,ui->compileAction,true));
    connect(ChainIDE::getInstance()->getCompileManager(),&CompileManager::errorCompile,std::bind(&QAction::setEnabled,ui->compileAction,true));

    //设置左侧文件树的一些槽
    connect(ui->fileWidget,&FileWidget::fileClicked,ui->contentWidget,&ContextWidget::showFile);
    connect(ui->fileWidget,&FileWidget::compileFile,this,&MainWindow::on_compileAction_triggered);
    connect(ui->fileWidget,&FileWidget::deleteFile,ui->contentWidget,&ContextWidget::CheckDeleteFile);
    connect(ui->fileWidget,&FileWidget::newFile,this,&MainWindow::NewFile);
    connect(ui->fileWidget,&FileWidget::fileClicked,this,&MainWindow::ModifyActionState);
    connect(ui->fileWidget,&FileWidget::dirClicked,this,&MainWindow::ModifyActionState);

    //设置中间代码编辑框的一些槽
    connect(ui->contentWidget,&ContextWidget::fileSelected,ui->fileWidget,&FileWidget::SelectFile);
    connect(ui->contentWidget,&ContextWidget::contentStateChange,this,&MainWindow::ModifyActionState);
    connect(ui->contentWidget,&ContextWidget::GetBreakPointFinish,ChainIDE::getInstance()->getDebugManager(),&DebugManager::fetchBreakPointsFinish);

    //已注册合约
    connect(ui->tabWidget,&QTabWidget::currentChanged,this,&MainWindow::tabWidget_currentChanged);

    //调试器调试开始通知、完成、出错
    connect(ChainIDE::getInstance()->getDebugManager(),&DebugManager::debugStarted,[this](){
        this->ui->outputWidget->clearOutputMessage();
        this->ModifyDebugActionState();
    });
    connect(ChainIDE::getInstance()->getDebugManager(),&DebugManager::debugFinish,[this](){
        this->ui->contentWidget->ClearDebuggerLine(ChainIDE::getInstance()->getDebugManager()->getCurrentDebugFile());
        this->ui->debugWidget->setVisible(false);
        this->ModifyActionState();

    });
    connect(ChainIDE::getInstance()->getDebugManager(),&DebugManager::debugError,[this](){
        this->ui->contentWidget->ClearDebuggerLine(ChainIDE::getInstance()->getDebugManager()->getCurrentDebugFile());
        this->ui->debugWidget->setVisible(false);
        this->ModifyActionState();
    });

    //调获取断点信息、断点停顿通知
    connect(ChainIDE::getInstance()->getDebugManager(),&DebugManager::fetchBreakPoints,ui->contentWidget,&ContextWidget::GetBreakPointSlots);
    connect(ChainIDE::getInstance()->getDebugManager(),&DebugManager::debugBreakAt,ui->contentWidget,&ContextWidget::SetDebuggerLine);

    //调试后台输出通知
    connect(ChainIDE::getInstance()->getDebugManager(),&DebugManager::debugOutput,ui->outputWidget,&OutputWidget::receiveOutputMessage);

    //调试变量、堆栈更新通知
    connect(ChainIDE::getInstance()->getDebugManager(),&DebugManager::variantUpdated,ui->debugWidget,&DebugWidget::UpdateInfoData);
    connect(ChainIDE::getInstance()->getDebugManager(),&DebugManager::backTraceUpdated,ui->debugWidget,&DebugWidget::UpdateBackTrace);

    //调试器调整断点位置通知
    connect(ChainIDE::getInstance()->getDebugManager(),&DebugManager::addBreakPoint,ui->contentWidget,&ContextWidget::AddBreakPoint);
    connect(ChainIDE::getInstance()->getDebugManager(),&DebugManager::removeBreakPoint,ui->contentWidget,&ContextWidget::RemoveBreakPoint);

    //设置调试窗口双击堆栈跳转到编辑器对应行
    connect(ui->debugWidget,&DebugWidget::JumpToLine,std::bind(&ContextWidget::JumpToLine,ui->contentWidget,
                std::ref(ChainIDE::getInstance()->getDebugManager()->getCurrentDebugFile()),std::placeholders::_1,0));

    //初始化后台消息展示框
    initMessageWidget();

    //调整按钮状态
    ModifyActionState();

    //状态栏开始更新
    ui->statusBar->startStatus();
}

void MainWindow::refreshTitle()
{
    if(ChainIDE::getInstance()->getCurrentChainType() == DataDefine::NONE)
    {
        setWindowTitle(tr("IDE"));
    }
    else if(ChainIDE::getInstance()->getCurrentChainType() == DataDefine::TEST)
    {
        if(ChainIDE::getInstance()->getChainClass() == DataDefine::UB)
        {
            setWindowTitle(tr("IDE-UB TEST CHAIN"));
        }
        else if(ChainIDE::getInstance()->getChainClass() == DataDefine::HX)
        {
            setWindowTitle(tr("IDE-HX TEST CHAIN"));
        }
        else if(ChainIDE::getInstance()->getChainClass() == DataDefine::CTC)
        {
            setWindowTitle(tr("IDE-CTC TEST CHAIN"));
        }
    }
    else if(ChainIDE::getInstance()->getCurrentChainType() == DataDefine::FORMAL)
    {
        if(ChainIDE::getInstance()->getChainClass() == DataDefine::UB)
        {
            setWindowTitle(tr("IDE-UB FORMAL CHAIN"));
        }
        else if(ChainIDE::getInstance()->getChainClass() == DataDefine::HX)
        {
            setWindowTitle(tr("IDE-HX FORMAL CHAIN"));
        }
        else if(ChainIDE::getInstance()->getChainClass() == DataDefine::CTC)
        {
            setWindowTitle(tr("IDE-CTC FORMAL CHAIN"));
        }
    }

}

void MainWindow::refreshStyle()
{
    //初始化样式表
    ChainIDE::getInstance()->refreshStyleSheet();
}

void MainWindow::refreshTranslator()
{
    ChainIDE::getInstance()->refreshTranslator();
    ui->retranslateUi(this);
    refreshTitle();

    ui->fileWidget->retranslator();
    ui->outputWidget->retranslator();
    ui->contractWidget->retranslator();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //关闭窗口是先判断编辑窗是否要关闭
    if(!ui->contentWidget->closeAll())
    {
        event->ignore();
        return;
    }
//正式开始关闭操作
    //发送关闭窗口信号，其他额外添加的关闭操作可连接此信号进行关闭，例如添加的后台消息输出框
    emit windowClose();
    //隐藏自身，防止用户因为关闭太慢以为ide死掉了
    hide();
    //先关闭后台链
    if((ChainIDE::getInstance()->getStartChainTypes() | DataDefine::NONE) ||
       (ChainIDE::getInstance()->getBackStageManager()->isBackStageRunning()))
    {
        CommonDialog dia(CommonDialog::NONE);
        dia.setText(tr("请耐心等待程序自动关闭，不要关闭本窗口!"));
        ui->statusBar->stopStatus();
        connect(ChainIDE::getInstance()->getBackStageManager(),&BackStageManager::closeBackStageFinish,&dia,&CommonDialog::close);
        ChainIDE::getInstance()->getBackStageManager()->closeBackStage();

        dia.exec();
    }
    //关闭调试器
    ChainIDE::getInstance()->getDebugManager()->ReadyClose();
    //判断是否需要更新重启---github无此需求，之前热更新有该需求
    if(_p->updateNeeded)
    {//开启copy，
        QString updateExe = QCoreApplication::applicationDirPath()+"/Update";
        QString package = "update.zip";
        QString mainName = QCoreApplication::applicationName();
        QString unpackName = "update";//解压后的文件夹名称
        QString tempName = "updatetemp";//临时路径
        if(QFileInfo(updateExe).exists())
        {
        #ifndef WIN32
            QProcess::execute("chmod",QStringList()<<"777"<<updateExe);
        #endif
            QProcess *copproc = new QProcess();
            copproc->startDetached(updateExe,QStringList()<<package<<mainName<<unpackName<<tempName);
        }
        else
        {
            CommonDialog dia(CommonDialog::OkOnly);
            dia.setText(tr("未找到Update更新程序包!"));
            dia.exec();
        }
    }
    //调用父类关闭其他默认内容
    QMainWindow::closeEvent(event);
}

void MainWindow::exeStartedSlots()
{
    //初始化数据管理
    if(ChainIDE::getInstance()->getChainClass() == DataDefine::HX)
    {
        DataManagerHX::getInstance()->InitManager();
        DataManagerHX::getInstance()->dealNewState();//处理最新情况
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::UB)
    {
        DataManagerUB::getInstance()->InitManager();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::CTC)
    {
        DataManagerCTC::getInstance()->InitManager();
        DataManagerCTC::getInstance()->dealNewState();
    }

    //关闭等待窗
    emit initFinish();
    //启动主窗口
    startWidget();

}

void MainWindow::exeErrorSlots()
{
    emit exeError();
    //后台出错，请关闭本窗口
    ConvenientOp::ShowNotifyMessage(tr("Backstage error! Please check the chain!"));
    //
    if(this->isHidden())
    {
        startWidget();
    }
    else
    {
        updateNeededSlot(false);
//        close();
    }
}

void MainWindow::showWaitingForSyncWidget()
{
    WaitingForSync *waitingForSync = new WaitingForSync();
    waitingForSync->setAttribute(Qt::WA_DeleteOnClose);
    connect(this,&MainWindow::initFinish,std::bind(&WaitingForSync::ReceiveMessage,waitingForSync,tr("Initialize done..."),0));
    connect(this,&MainWindow::initFinish,waitingForSync,&WaitingForSync::close);
    connect(this,&MainWindow::exeError,waitingForSync,&WaitingForSync::close);
    connect(ChainIDE::getInstance()->getBackStageManager(),&BackStageManager::OutputMessage,waitingForSync,&WaitingForSync::ReceiveMessage);

    waitingForSync->setWindowTitle( QString::fromLocal8Bit("IDE"));

    waitingForSync->ReceiveMessage(tr("Initialize IDE,please wait..."));

    waitingForSync->show();
}

void MainWindow::initMessageWidget()
{
    if(_p->backstageMessageShow) return;
    _p->backstageMessageShow = new MessageTypeShowWidget();
    //消息匹配框
    static const std::map<DataDefine::BackStageMessageType,QString> &messTypeMap = {
        {DataDefine::NONE_TYPE,""},
        {DataDefine::NODE_ERROR_TEST_TYPE,"node_1"} ,{DataDefine::NODE_OUT_TEST_TYPE,"node_1"} ,
        {DataDefine::NODE_ERROR_FORMAL_TYPE,"node_2"} ,{DataDefine::NODE_OUT_FORMAL_TYPE,"node_2"} ,
        {DataDefine::CLIENT_ERROR_TEST_TYPE,"cli_1"} ,{DataDefine::CLIENT_OUT_TEST_TYPE,"cli_1"} ,
        {DataDefine::CLIENT_ERROR_FORMAL_TYPE,"cli_2"} ,{DataDefine::CLIENT_OUT_FORMAL_TYPE,"cli_2"}
    };
    connect(ChainIDE::getInstance()->getBackStageManager(),&BackStageManager::OutputMessage,[this](const QString &message,int messageType){
        this->_p->backstageMessageShow->ReceiveMessage(message,messTypeMap.at(static_cast<DataDefine::BackStageMessageType>(messageType)));
    });
    connect(this,&MainWindow::windowClose,_p->backstageMessageShow,&MessageTypeShowWidget::close);
}

void MainWindow::on_newContractAction_glua_triggered()
{//新建glua合约，只能放在glua文件夹下
    NewFile(DataDefine::GLUA_SUFFIX);
}

void MainWindow::on_newContractAction_csharp_triggered()
{//新建csharp合约，放在csharp目录下
    NewFile(DataDefine::CSHARP_SUFFIX);
}

void MainWindow::on_newContractAtion_kotlin_triggered()
{//新建kotlin合约，只能放在kotlin文件夹下
    NewFile(DataDefine::KOTLIN_SUFFIX);
}

void MainWindow::on_newContractAction_java_triggered()
{//新建java合约，放在java目录下
    NewFile(DataDefine::JAVA_SUFFIX);
}

void MainWindow::on_importContractAction_glua_triggered()
{
    ConvenientOp::ImportContractFile(QCoreApplication::applicationDirPath()+"/"+DataDefine::GLUA_DIR);
}

void MainWindow::on_importContractAction_java_triggered()
{
    ConvenientOp::ImportContractFile(QCoreApplication::applicationDirPath()+"/"+DataDefine::JAVA_DIR);
}

void MainWindow::on_importContractAction_csharp_triggered()
{
    ConvenientOp::ImportContractFile(QCoreApplication::applicationDirPath()+"/"+DataDefine::CSHARP_DIR);
}

void MainWindow::on_importContractAction_kotlin_triggered()
{
    ConvenientOp::ImportContractFile(QCoreApplication::applicationDirPath()+"/"+DataDefine::KOTLIN_DIR);
}

void MainWindow::on_saveAction_triggered()
{
    ui->contentWidget->saveFile();
}

void MainWindow::on_savaAsAction_triggered()
{

}

void MainWindow::on_saveAllAction_triggered()
{
    ui->contentWidget->saveAll();
}

void MainWindow::on_closeAction_triggered()
{
    ui->contentWidget->closeFile(ui->contentWidget->getCurrentFilePath());
}

void MainWindow::on_closeAllAction_triggered()
{
    ui->contentWidget->closeAll();
}

void MainWindow::tabWidget_currentChanged(int index)
{
    //左侧tab页面切换到合约界面时，需要刷新一下合约树
    if(index == 1)
    {
        ui->contractWidget->RefreshTree();
    }
}

void MainWindow::HideAction()
{
    //禁用右键
    this->setContextMenuPolicy(Qt::NoContextMenu);
    //隐藏左右控制按钮
    ui->tabWidget->tabBar()->setUsesScrollButtons(false);

    //隐藏不需要的按钮
    ui->savaAsAction->setVisible(false);//另存为
    ui->enterSandboxAction->setVisible(false);//进入沙盒模式
    ui->exitSandboxAction->setVisible(false);//离开沙盒模式
    ui->withdrawAction->setVisible(false);//销毁合约
    ui->importAction->setVisible(false);//导入合约
    ui->exportAction->setVisible(false);//到处合约

    //启动链引起的按钮显示不可用
    //切换正式、测试链--仅在同时加载了测试、正式链才会显示
    ui->changeChainAction->setVisible((ChainIDE::getInstance()->getStartChainTypes() & DataDefine::TEST) &&
                                      (ChainIDE::getInstance()->getStartChainTypes() & DataDefine::FORMAL));
    //合约界面，不启动链，则不显示链上合约
    if(ChainIDE::getInstance()->getStartChainTypes() == DataDefine::NONE)
    {
        ui->tabWidget->removeTab(1);
    }
    //调用合约、注册合约、账户、转账、控制台、后台输出按钮，不启动链的时候，都不显示
    ui->callAction->setEnabled(ChainIDE::getInstance()->getStartChainTypes() != DataDefine::NONE);
    ui->registerAction->setEnabled(ChainIDE::getInstance()->getStartChainTypes() != DataDefine::NONE);
    ui->accountListAction->setEnabled(ChainIDE::getInstance()->getStartChainTypes() != DataDefine::NONE);
    ui->transferToAccountAction->setEnabled(ChainIDE::getInstance()->getStartChainTypes() != DataDefine::NONE);
    ui->consoleAction->setEnabled(ChainIDE::getInstance()->getStartChainTypes() != DataDefine::NONE);
    ui->BackStageMessageAction->setEnabled(ChainIDE::getInstance()->getStartChainTypes() != DataDefine::NONE);
    //升级合约、转账到合约，只有HX、CTC链支持，，，，UB这玩意有点复杂，先不支持了
    ui->upgradeAction->setEnabled(ChainIDE::getInstance()->getStartChainTypes() != DataDefine::NONE &&
                                  (ChainIDE::getInstance()->getChainClass() == DataDefine::HX || ChainIDE::getInstance()->getChainClass() == DataDefine::CTC));
    ui->transferAction->setEnabled(ChainIDE::getInstance()->getStartChainTypes() != DataDefine::NONE &&
                                  (ChainIDE::getInstance()->getChainClass() == DataDefine::HX || ChainIDE::getInstance()->getChainClass() == DataDefine::CTC));

}

void MainWindow::on_configAction_triggered()
{//配置界面
    ConfigWidget config;
    if(config.pop())
    {
        //语言修改时、默认自动刷新，，其他的需要重启，比如样式表，，无法在不重新加载的时候更换，因为已经打开的编辑框无法更换
        refreshTranslator();
//        refreshStyle();
    }

}

void MainWindow::on_exitAction_triggered()
{
    //关闭操作
    close();
}

void MainWindow::on_undoAction_triggered()
{
    ui->contentWidget->undo();
}

void MainWindow::on_redoAction_triggered()
{
    ui->contentWidget->redo();
}

void MainWindow::on_importAction_triggered()
{

}

void MainWindow::on_exportAction_triggered()
{

}

void MainWindow::on_registerAction_triggered()
{
    if(ChainIDE::getInstance()->getChainClass() == DataDefine::HX)
    {
        RegisterContractDialogHX dia;
        dia.exec();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::UB)
    {
        RegisterContractDialogUB dia;
        dia.exec();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::CTC)
    {
        RegisterContractDialogCTC dia;
        dia.exec();
    }
}

void MainWindow::on_transferAction_triggered()
{
    if(ChainIDE::getInstance()->getChainClass() == DataDefine::HX)
    {
        TransferToContractHX dia;
        dia.exec();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::UB)
    {
//        TransferToContractUB dia;
//        dia.exec();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::CTC)
    {
        TransferToContractCTC dia;
        dia.exec();
    }
}

void MainWindow::on_callAction_triggered()
{//调用合约
    if(ChainIDE::getInstance()->getChainClass() == DataDefine::HX)
    {
        CallContractWidgetHX callWidget;
        callWidget.exec();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::UB)
    {
        CallContractWidgetUB callWidget;
        callWidget.exec();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::CTC)
    {
        CallContractWidgetCTC callWidget;
        callWidget.exec();
    }
}

void MainWindow::on_upgradeAction_triggered()
{
    if(ChainIDE::getInstance()->getChainClass() == DataDefine::HX)
    {
        UpgradeContractDialogHX upgradeContractWidget;
        upgradeContractWidget.exec();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::UB)
    {

    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::CTC)
    {
        UpgradeContractDialogCTC upgradeContractWidget;
        upgradeContractWidget.exec();
    }
}

void MainWindow::on_withdrawAction_triggered()
{

}

void MainWindow::on_changeChainAction_triggered()
{
    ChainIDE::getInstance()->setCurrentChainType(static_cast<DataDefine::ChainType>(ChainIDE::getInstance()->getCurrentChainType() ^ ChainIDE::getInstance()->getStartChainTypes()));

    if(ChainIDE::getInstance()->getCurrentChainType() == DataDefine::FORMAL)
    {
        CommonDialog dia(CommonDialog::OkAndCancel);
        dia.setText(tr("Sure to switch to formal chain?"));
        if(dia.pop())
        {
            if(ChainIDE::getInstance()->getChainClass() == DataDefine::HX)
            {
                DataManagerHX::getInstance()->dealNewState();//处理最新情况
            }
            else if(ChainIDE::getInstance()->getChainClass() == DataDefine::CTC)
            {
                DataManagerCTC::getInstance()->dealNewState();//处理最新情况
            }
        }
        else
        {
            ChainIDE::getInstance()->setCurrentChainType(DataDefine::TEST);
        }
    }
    refreshTitle();
    ModifyActionState();
}

void MainWindow::on_compileAction_triggered()
{//编译
    //先触发保存判断
    if( ui->contentWidget->currentFileUnsaved() && ui->contentWidget->getCurrentFilePath() == ui->fileWidget->getCurrentFile())
    {
        QMessageBox::StandardButton choice = QMessageBox::information(nullptr, "", ui->contentWidget->getCurrentFilePath() + " " + tr("文件已修改，是否保存?"), QMessageBox::Yes | QMessageBox::No);
        if( QMessageBox::Yes == choice)
        {
            ui->contentWidget->saveFile();
        }
        else
        {
            return;
        }
    }

//  清空编译输出窗口
    ui->outputWidget->clearCompileMessage();
    ui->compileAction->setEnabled(false);
    ChainIDE::getInstance()->getCompileManager()->startCompile(ui->fileWidget->getCurrentFile());//当前打开的文件

}

void MainWindow::on_debugAction_triggered()
{
#ifdef DEBUG_FUNC_OFF
    ConvenientOp::ShowNotifyMessage(tr("Debug function is in progress!"));
    return;
#endif
    if(ChainIDE::getInstance()->getDebugManager()->getDebuggerState() == DebugDataStruct::Available)
    {
        //先生成.out字节码
        if(ui->compileAction->isEnabled())
        {
            connect(ChainIDE::getInstance()->getCompileManager(),&CompileManager::finishCompile,this,&MainWindow::startDebugSlot);
            connect(ChainIDE::getInstance()->getCompileManager(),&CompileManager::errorCompile,[this](){
                disconnect(ChainIDE::getInstance()->getCompileManager(),&CompileManager::finishCompile,this,&MainWindow::startDebugSlot);
            });
            on_compileAction_triggered();

        }
    }
    else
    {
        ChainIDE::getInstance()->getDebugManager()->debugContinue();
    }
}

void MainWindow::startDebugSlot(const QString &gpcFile)
{
    disconnect(ChainIDE::getInstance()->getCompileManager(),&CompileManager::finishCompile,this,&MainWindow::startDebugSlot);

    //获取调试函数、参数
    DebugFunctionWidget fun(ui->fileWidget->getCurrentFile(),gpcFile);
    fun.exec();
    if(fun.SelectedApi().isEmpty()) return;

    //显示调试窗
    ui->debugWidget->ClearData();
    ui->debugWidget->setVisible(true);
    //打开调试器，进行函数调试
    QString outfile = gpcFile;
    outfile.replace(QRegExp(DataDefine::CONTRACT_SUFFIX+"$"),DataDefine::BYTE_OUT_SUFFIX);
    ChainIDE::getInstance()->getDebugManager()->startDebug(ui->fileWidget->getCurrentFile(),outfile,fun.SelectedApi(),fun.ApiParams());
}

void MainWindow::on_stopAction_triggered()
{
    ChainIDE::getInstance()->getDebugManager()->stopDebug();
}

void MainWindow::on_stepAction_triggered()
{
    ChainIDE::getInstance()->getDebugManager()->debugNextStep();
}

void MainWindow::on_stepInAction_triggered()
{
    ChainIDE::getInstance()->getDebugManager()->debugStepInto();
}

void MainWindow::on_TabBreaPointAction_triggered()
{//设置断点
    ui->contentWidget->TabBreakPoint();
}

void MainWindow::on_DeleteAllBreakpointAction_triggered()
{//清空断点
    ui->contentWidget->ClearBreakPoint();
}

void MainWindow::on_accountListAction_triggered()
{
    if(ChainIDE::getInstance()->getChainClass() == DataDefine::HX)
    {
        AccountWidgetHX account;
        account.exec();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::UB)
    {
        AccountWidgetUB account;
        account.exec();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::CTC)
    {
        AccountWidgetCTC account;
        account.exec();
    }
}

void MainWindow::on_consoleAction_triggered()
{
    static ConsoleDialog *consoleDialog = nullptr;
    if(!consoleDialog)
    {
        consoleDialog = new ConsoleDialog();
        connect(this,&MainWindow::windowClose,consoleDialog,&ConsoleDialog::close);
    }
    if(consoleDialog)
    {
        consoleDialog->show();
    }

}

void MainWindow::on_transferToAccountAction_triggered()
{//转账
    if(ChainIDE::getInstance()->getChainClass() == DataDefine::HX)
    {
        TransferWidgetHX transfer;
        transfer.exec();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::UB)
    {
        TransferWidgetUB transfer;
        transfer.exec();
    }
    else if(ChainIDE::getInstance()->getChainClass() == DataDefine::CTC)
    {
        TransferWidgetCTC transfer;
        transfer.exec();
    }

}

void MainWindow::on_BackStageMessageAction_triggered()
{
    if(_p->backstageMessageShow)
    {
        _p->backstageMessageShow->show();
    }
}

void MainWindow::on_contractHelpAction_triggered()
{
    //打开合约帮助网页
    QDesktopServices::openUrl(QUrl(QString("file:///%1/%2").arg(QCoreApplication::applicationDirPath()).arg(DataDefine::CONTRACT_HELP_PATH)));
}

void MainWindow::on_editorHelpAction_triggered()
{
    //打开编辑器帮助网页
    QString fileName = DataDefine::EDITOR_HELP_PATH_CH;
    switch (ChainIDE::getInstance()->getCurrentLanguage()) {
    case DataDefine::SimChinese:
        fileName = DataDefine::EDITOR_HELP_PATH_CH;
        break;
    case DataDefine::English:
        fileName = DataDefine::EDITOR_HELP_PATH_ENG;
        break;
    default:
        fileName = DataDefine::EDITOR_HELP_PATH_CH;
        break;
    }
    QDesktopServices::openUrl(QUrl(QString("file:///%1/%2").arg(QCoreApplication::applicationDirPath()).arg(fileName)));
}

void MainWindow::on_aboutAction_triggered()
{//关于界面
//    AboutWidget about;
//    connect(&about,&AboutWidget::RestartSignal,this,&MainWindow::close);
//    connect(&about,&AboutWidget::UpdateNeeded,this,&MainWindow::updateNeededSlot);
//    about.exec();
    AboutWidgetGit about;
    about.exec();
}

void MainWindow::ModifyActionState()
{
    //切换正式、测试链的按钮状态
    if(ui->changeChainAction->isVisible())
    {
        ui->changeChainAction->setIcon(ChainIDE::getInstance()->getCurrentChainType()==DataDefine::TEST?
                                       QIcon(":/pic/changeToFormal_enable.png"):QIcon(":/pic/changeToTest_enable.png"));
        ui->changeChainAction->setText(ChainIDE::getInstance()->getCurrentChainType()==DataDefine::TEST?tr("切换到正式链"):tr("切换到测试链"));
    }
    //编辑菜单功能
    ui->undoAction->setEnabled(ui->contentWidget->isUndoAvailable());
    ui->redoAction->setEnabled(ui->contentWidget->isRedoAvailable());
    ui->saveAction->setEnabled(ui->contentWidget->currentFileUnsaved());
    ui->saveAllAction->setEnabled(ui->contentWidget->hasFileUnsaved());
    ui->closeAction->setEnabled(!ui->contentWidget->getCurrentFilePath().isEmpty());
    ui->closeAllAction->setEnabled(!ui->contentWidget->getCurrentFilePath().isEmpty());
    ui->savaAsAction->setEnabled(ui->fileWidget->getCurrentFile().isEmpty());

    //编译功能
    QString currentFile = ui->fileWidget->getCurrentFile();
    if(QFileInfo(currentFile).isFile()&&
       (currentFile.endsWith(DataDefine::GLUA_SUFFIX)||
       currentFile.endsWith(DataDefine::JAVA_SUFFIX)||
       currentFile.endsWith(DataDefine::CSHARP_SUFFIX)||
       currentFile.endsWith(DataDefine::KOTLIN_SUFFIX)))
    {
        ui->compileAction->setEnabled(true);

    }
    else
    {
        ui->compileAction->setEnabled(false);
    }

    //调试按钮单独设置
    ModifyDebugActionState();
}

void MainWindow::ModifyDebugActionState()
{
    QString currentFile = ui->fileWidget->getCurrentFile();
    if(QFileInfo(currentFile).isFile()&&
       (currentFile.endsWith(DataDefine::GLUA_SUFFIX)||
       currentFile.endsWith(DataDefine::JAVA_SUFFIX)||
       currentFile.endsWith(DataDefine::CSHARP_SUFFIX)||
       currentFile.endsWith(DataDefine::KOTLIN_SUFFIX)))
    {
        ui->debugAction->setEnabled(true);
    }
    else
    {
        ui->debugAction->setEnabled(ChainIDE::getInstance()->getDebugManager()->getDebuggerState() != DebugDataStruct::Available);
    }
    ui->stepAction->setEnabled(ChainIDE::getInstance()->getDebugManager()->getDebuggerState() != DebugDataStruct::Available);
    ui->stepInAction->setEnabled(ChainIDE::getInstance()->getDebugManager()->getDebuggerState() != DebugDataStruct::Available);
    ui->stopAction->setEnabled(ChainIDE::getInstance()->getDebugManager()->getDebuggerState() != DebugDataStruct::Available);

    QString contentFile = ui->contentWidget->getCurrentFilePath();
    if(QFileInfo(contentFile).isFile()&&
       (contentFile.endsWith(DataDefine::GLUA_SUFFIX)||
       contentFile.endsWith(DataDefine::JAVA_SUFFIX)||
       contentFile.endsWith(DataDefine::CSHARP_SUFFIX)||
       contentFile.endsWith(DataDefine::KOTLIN_SUFFIX)))
    {
        ui->TabBreaPointAction->setEnabled(true);
        ui->DeleteAllBreakpointAction->setEnabled(true);
    }
    else
    {
        ui->TabBreaPointAction->setEnabled(false);
        ui->DeleteAllBreakpointAction->setEnabled(false);
    }
}

void MainWindow::NewFile(const QString &suffix ,const QString &defaultPath)
{
    QString topDir;
    QStringList suffixs;
    if(suffix == DataDefine::GLUA_SUFFIX)
    {
        topDir = QCoreApplication::applicationDirPath()+"/"+DataDefine::GLUA_DIR;
        suffixs<<"."+DataDefine::GLUA_SUFFIX;
    }
    else if(suffix == DataDefine::JAVA_SUFFIX)
    {
        topDir = QCoreApplication::applicationDirPath()+"/"+DataDefine::JAVA_DIR;
        suffixs<<"."+DataDefine::JAVA_SUFFIX;

    }
    else if(suffix == DataDefine::CSHARP_SUFFIX)
    {
        topDir = QCoreApplication::applicationDirPath()+"/"+DataDefine::CSHARP_DIR;
        suffixs<<"."+DataDefine::CSHARP_SUFFIX;

    }
    else if(suffix == DataDefine::KOTLIN_SUFFIX)
    {
        topDir = QCoreApplication::applicationDirPath()+"/"+DataDefine::KOTLIN_DIR;
        suffixs<<"."+DataDefine::KOTLIN_SUFFIX;
    }
    NewFileDialog dia(topDir,suffixs,defaultPath);
    dia.exec();
    NewFileCreated(dia.getNewFilePath());
}

void MainWindow::updateNeededSlot(bool is)
{
    _p->updateNeeded = is;
}

void MainWindow::NewFileCreated(const QString &filePath)
{//新建了文件后，文件树刷新，点击文件树，
    if(filePath.isEmpty() || !QFileInfo(filePath).isFile()) return;
    ui->fileWidget->SelectFile(filePath);
    ui->contentWidget->showFile(filePath);
}




