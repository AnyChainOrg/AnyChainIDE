#ifndef WEBSOCKETREQUIRE_H
#define WEBSOCKETREQUIRE_H

#include "RequireBase.h"
#include <QAbstractSocket>
//websocket请求
class websocketRequire : public RequireBase
{
    Q_OBJECT
public:
    explicit websocketRequire(const QString &ip,const QString & connectPort,QObject *parent = nullptr);
    ~websocketRequire()override final;
public:
    void postData(const QString &data) override final;
    void startConnect() override final;
    bool isConnected()const override final;

private slots:
    void onTextFrameReceived(const QString &,bool);
    void onStateChanged(QAbstractSocket::SocketState);
private:
    void Init();
private:
    class DataPrivate;
    DataPrivate *_p;
};

#endif // WEBSOCKETREQUIRE_H
