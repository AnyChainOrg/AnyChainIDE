#include "AboutWidgetGit.h"
#include "ui_AboutWidgetGit.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#include <QDesktopServices>
#include "update/UpdateProgressUtil.h"

static const QString IDE_VERSION = "1.0.13";
static const QString UPDATE_SERVER_URL = "https://github.com/AnyChainOrg/AnyChainIDE/releases";
#ifdef WIN32
static const QString VERSION_DOWNLOAD_PATTERN = "/AnyChainOrg/AnyChainIDE/releases/download/v(\\d+\\.\\d+\\.\\d+)/([0-9a-zA-Z_]+\\.zip)";
#elif defined(TARGET_OS_MAC)
static const QString VERSION_DOWNLOAD_PATTERN = "/AnyChainOrg/AnyChainIDE/releases/download/v(\\d+\\.\\d+\\.\\d+)/([0-9a-zA-Z_]+\\.dmg)";
#endif
class AboutWidgetGit::DataPrivate
{
public:
    DataPrivate()
        :networkManager(new QNetworkAccessManager())
    {

    }
    QNetworkAccessManager *networkManager;
};

AboutWidgetGit::AboutWidgetGit(QWidget *parent) :
    MoveableDialog(parent),
    ui(new Ui::AboutWidgetGit),
    _p(new DataPrivate())
{
    ui->setupUi(this);
    InitWidget();
}

AboutWidgetGit::~AboutWidgetGit()
{
    delete _p;
    delete ui;
}

void AboutWidgetGit::checkNewVersion()
{
    ui->label_updatetip->setText(tr("Start check version, please wait..."));
    ui->label_updatetip->setVisible(true);
    QNetworkReply *reply = _p->networkManager->get(QNetworkRequest(UPDATE_SERVER_URL));
    connect(reply,static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),[this](){
        QString text = tr("Check update wrong! Please manually check update, click here." );
        this->ui->label_updatetip->setText(tr("<a style='color: red;'href=\"%1\">%2").arg(UPDATE_SERVER_URL).arg(text));

        this->ui->label_updatetip->setText(tr("Check update wrong!"));
//        this->ui->label_updatetip->setVisible(true);
    });
    connect(reply,&QNetworkReply::finished,[reply,this](){
        QString result(reply->readAll());
        QRegExp rx(VERSION_DOWNLOAD_PATTERN);
        if(result.indexOf(rx) >= 0)
        {
            QString serverVersion(rx.cap(1));
            if(UpdateProgressUtil::AFTER!=UpdateProgressUtil::CompareVersion(IDE_VERSION,serverVersion))
            {
                this->ui->label_updatetip->setText(tr("No new version!"));
            }
            else
            {
                QString text = tr("New version found! v" )+ serverVersion;
                this->ui->label_updatetip->setText(tr("<a style='color: red;'href=\"%1\">%2").arg(UPDATE_SERVER_URL).arg(text));
            }
        }
        else
        {
            this->ui->label_updatetip->setText(tr("No release found!"));
        }
//        this->ui->label_updatetip->setVisible(true);
    });


}

void AboutWidgetGit::InitWidget()
{
    setWindowFlags(windowFlags()| Qt::FramelessWindowHint);
    ui->label_updatetip->setVisible(false);
#ifdef WIN64
    ui->label_version->setText(QString("windows 64bit v") + IDE_VERSION);
#elif defined(TARGET_OS_MAC)
    ui->label_version->setText(QString("mac v") + IDE_VERSION);
#else
    ui->label_version->setText(QString("windows 32bit v") + IDE_VERSION);
#endif
    connect(ui->closeBtn,&QToolButton::clicked,this,&AboutWidgetGit::close);
    connect(ui->toolButton_checkUpdate,&QToolButton::clicked,this,&AboutWidgetGit::checkNewVersion);

    ui->label_updatetip->setOpenExternalLinks(true);
}
