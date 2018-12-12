#include "DebugFunctionWidget.h"
#include "ui_DebugFunctionWidget.h"

#include <vector>
#include <QDebug>
#include <QProcess>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QCoreApplication>
#include <QFileInfo>
#include "ConvenientOp.h"

class DebugFunctionWidget::DataPrivate
{
public:
    DataPrivate(const QString &source,const QString &gpc)
        :allApi(nullptr)
        ,sourcefile(source)
        ,gpcFile(gpc)
        ,storageFile(QCoreApplication::applicationDirPath()+"/"+DataDefine::DEBUGGER_UVM_DIR+"/"+DataDefine::STORAGE_DATA_FILE_NAME)
    {
        //meta.json
        metaFile = gpcFile;
        metaFile.replace(QRegExp(DataDefine::CONTRACT_SUFFIX+"$"),DataDefine::META_SUFFIX);
        //.out
        outFile = gpcFile;
        outFile.replace(QRegExp(DataDefine::CONTRACT_SUFFIX+"$"),DataDefine::BYTE_OUT_SUFFIX);
        //function-param-template
        functionParamTemplate.emplace_back(std::make_pair("on_deposit",std::make_pair("[0-9]*","0")));
        functionParamTemplate.emplace_back(std::make_pair("on_deposit_asset",std::make_pair("^{\"symbol\":\"[a-zA-A]*\",\"num\":[0-9]*,\"param\":\"[a-zA-Z0-9_]*\"}$","{\"symbol\":\"HX\",\"num\":0,\"param\":\"none\"}")));
    }
public:
    QString sourcefile;
    QString gpcFile;
    QString metaFile;
    QString outFile;
    DataDefine::ApiEventPtr allApi;
    QString api;
    QStringList params;

    QString storageFile;//uvm数据内容
    std::vector<std::pair<QString,std::pair<QString,QString>>> functionParamTemplate;//函数参数模板,function+正则+默认参数
};

DebugFunctionWidget::DebugFunctionWidget(const QString &sourcefile,const QString &gpcFile,QWidget *parent) :
    MoveableDialog(parent),
    ui(new Ui::DebugFunctionWidget),
    _p(new DataPrivate(sourcefile,gpcFile))
{
    ui->setupUi(this);
    InitWidget();
}

DebugFunctionWidget::~DebugFunctionWidget()
{
    delete ui;
}

const QString &DebugFunctionWidget::SelectedApi() const
{
    return _p->api;
}

const QStringList &DebugFunctionWidget::ApiParams() const
{
    return _p->params;
}

void DebugFunctionWidget::OnOKClicked()
{
    //判断是否有storage数据，如果没有或者没有对应本文件的内容，则执行init函数
    if(!QFileInfo(_p->storageFile).exists())
    {
        ResetStorageData();
    }
    else
    {
        //读取文件内容
        QFile file(_p->storageFile);
        file.open(QIODevice::ReadOnly);
        QString context = file.readAll();
        file.close();
        if(!context.contains(_p->outFile))
        {
            ResetStorageData();
        }
    }

    //选择函数、参数，参数需要给默认调用者地址、pubkey
    _p->params.clear();
    _p->api.clear();
    if(!ui->function->currentData().value<QString>().isEmpty())
    {
        _p->api = ui->function->currentText();
    }
    if(ui->param->text().trimmed().isEmpty())
    {
        _p->params<<" ";
    }
    else
    {
        _p->params = ui->param->text().trimmed().split(" ");
    }
    _p->params<<DataDefine::DEBUG_TEST_ADDRESS<<DataDefine::DEBUG_TEST_PUBKEY;
    close();
}

void DebugFunctionWidget::OnResetClicked()
{
    ResetStorageData();
}

void DebugFunctionWidget::InitFunction()
{
    if(!ConvenientOp::readApiFromPath(_p->metaFile,_p->allApi)) return;

    QTreeWidget *tree = new QTreeWidget(this);
    tree->header()->setVisible(false);
    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList()<<"api");
    item->setFlags(Qt::ItemIsEnabled);
    tree->addTopLevelItem(item);

    QTreeWidgetItem *item1 = new QTreeWidgetItem(QStringList()<<"offline-api");
    item1->setFlags(Qt::ItemIsEnabled);
    tree->addTopLevelItem(item1);

    for(auto it = _p->allApi->getAllApiName().begin();it != _p->allApi->getAllApiName().end();++it)
    {
        QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList()<<*it);
        itemChild->setData(0,Qt::UserRole,QVariant::fromValue<QString>("api"));
        item->addChild(itemChild);
    }
    for(auto it = _p->allApi->getAllOfflineApiName().begin();it != _p->allApi->getAllOfflineApiName().end();++it)
    {
        QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList()<<*it);
        itemChild->setData(0,Qt::UserRole,QVariant::fromValue<QString>("offline-api"));
        item1->addChild(itemChild);
    }

    tree->expandAll();
    ui->function->setModel(tree->model());
    ui->function->setView(tree);
}

void DebugFunctionWidget::ResetStorageData()
{
    //先查找数据文件中，是否有该目录
    if(QFileInfo(_p->storageFile).exists())
    {
        QFile::remove(_p->storageFile);
    }
    QProcess pro;
    pro.setWorkingDirectory(QCoreApplication::applicationDirPath()+"/"+DataDefine::DEBUGGER_UVM_DIR);
    QStringList params(QStringList()<<"-k"<<_p->outFile<<"init"<<" "<<DataDefine::DEBUG_TEST_ADDRESS<<DataDefine::DEBUG_TEST_PUBKEY);
    qDebug()<<"init storage"<<QCoreApplication::applicationDirPath()+"/"+DataDefine::DEBUGGER_UVM_DIR+"/"+DataDefine::DEBUGGER_UVM_NAME<<params;
    pro.start(QCoreApplication::applicationDirPath()+"/"+DataDefine::DEBUGGER_UVM_DIR+"/"+DataDefine::DEBUGGER_UVM_NAME,
                params);
    pro.waitForFinished();
}

void DebugFunctionWidget::InitWidget()
{
    setWindowFlags(Qt::FramelessWindowHint);

    ui->file->setText(QFileInfo(_p->sourcefile).fileName());
    InitFunction();

    connect(ui->closeBtn,&QToolButton::clicked,this,&DebugFunctionWidget::close);
    connect(ui->cancelBtn,&QToolButton::clicked,this,&DebugFunctionWidget::close);
    connect(ui->okBtn,&QToolButton::clicked,this,&DebugFunctionWidget::OnOKClicked);
    connect(ui->param,&QLineEdit::returnPressed,this,&DebugFunctionWidget::OnOKClicked);
    connect(ui->resetDataBtn,&QToolButton::clicked,this,&DebugFunctionWidget::OnResetClicked);
    connect(ui->function,static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),[this](const QString &functionName){
        auto res = std::find_if(_p->functionParamTemplate.begin(),_p->functionParamTemplate.end(),[functionName](std::pair<QString,std::pair<QString,QString>> info){
                return info.first == functionName;});
        if( res != _p->functionParamTemplate.end())
        {
//            QRegExpValidator *va = new QRegExpValidator(QRegExp((*res).second.first),this->ui->param);
//            this->ui->param->setValidator(va);
            this->ui->param->setText((*res).second.second);
        }
        else
        {
//            this->ui->param->setValidator(nullptr);
            this->ui->param->clear();
        }
    });
}
