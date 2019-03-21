#include "DebuggerDataReuqire.h"

#include <mutex>
#include <QHostAddress>
#include <QTcpSocket>
#include <QJsonDocument>

class DebuggerDataReuqire::DataPrivate
{
public:
    DataPrivate()
        :socket(new QTcpSocket())
    {

    }
    ~DataPrivate()
    {
        if(socket)
        {
            socket->close();
            delete socket;
            socket = nullptr;
        }
    }

public:
    QTcpSocket* socket;//链接socket
    QString receiveData;//接收的内容缓存，防止出现分包情况，因此先缓存一部分
    std::mutex dataMutex;//接收内容修改锁

};
DebuggerDataReuqire::DebuggerDataReuqire(QObject *parent)
    : QObject(parent)
    ,_p(new DataPrivate())
{
    Init();
}

DebuggerDataReuqire::~DebuggerDataReuqire()
{
    delete _p;
    _p = nullptr;
}

void DebuggerDataReuqire::postData(const QString &data)
{
    if(!isConnected())
    {
        qDebug()<<"tcpsocket not connect to debugger. "<<data<<" abandoned";
        return;
    }
//    QByteArray senddata;
//    senddata.append("DEBB").append(reinterpret_cast<char*>(data.toUtf8().size()),4).append(data.toUtf8());
//    qDebug()<<"send to debugger:"<<senddata;
    _p->socket->write(data.toUtf8());
    _p->socket->waitForBytesWritten();
}

void DebuggerDataReuqire::startConnect(const QString &ip,const QString &port)
{
    _p->socket->connectToHost(QHostAddress(ip), static_cast<quint16>(port.toInt()));
    if (!_p->socket->waitForConnected()) {
        qDebug() << "could not connect to debugger: "<< _p->socket->errorString();
        emit connectFailed();
        return;
    }
    qDebug() << "connected to debugger: "<< ip<<":"<<port;
    emit connectFinish();
}

void DebuggerDataReuqire::disConnect()
{
    _p->socket->disconnectFromHost();
    _p->socket->waitForDisconnected();
}

bool DebuggerDataReuqire::isConnected() const
{
    return  QAbstractSocket::ConnectedState == _p->socket->state();
}

void DebuggerDataReuqire::readyReadSlots()
{
    appendReceiveData( _p->socket->readAll());
    setReceiveData(dealReceives(getReceiveData()));
}

void DebuggerDataReuqire::Init()
{
    connect(_p->socket,&QTcpSocket::readyRead,this,&DebuggerDataReuqire::readyReadSlots);
}

QString DebuggerDataReuqire::dealReceives(const QString &data)
{
    QString left("");
    QStringList strList=data.split("\n");
    foreach (QString param, strList) {
        // 判断是否接收到一个完整的json  完整的就退出read循环
        QJsonParseError json_error;
        QJsonDocument::fromJson(param.toUtf8(), &json_error);
        if(QJsonParseError::NoError == json_error.error)
        {
            emit receiveData(param);
        }
        else
        {
            left.append(param);
        }
    }
    return left;
}

void DebuggerDataReuqire::appendReceiveData(const QString &data)
{
    std::lock_guard<std::mutex> loc(_p->dataMutex);
    _p->receiveData.append(data);
}

void DebuggerDataReuqire::setReceiveData(const QString &data)
{
    std::lock_guard<std::mutex> loc(_p->dataMutex);
    _p->receiveData=data;
}

const QString &DebuggerDataReuqire::getReceiveData()const
{
    std::lock_guard<std::mutex> loc(_p->dataMutex);
    return _p->receiveData;
}
