#ifndef DEBUGGERDATAREUQIRE_H
#define DEBUGGERDATAREUQIRE_H

#include <QObject>
//调试器数据请求类
class DebuggerDataReuqire : public QObject
{
    Q_OBJECT
public:
    explicit DebuggerDataReuqire(QObject *parent = nullptr);
    ~DebuggerDataReuqire();
public:
    void postData(const QString &data) ;
    void startConnect(const QString &ip,const QString &port);
    void disConnect();
    bool isConnected()const ;
private slots:
    void readyReadSlots();
private:
    void Init();
signals:
    void receiveData(const QString &);
    void connectFinish();
    void connectFailed();
private:
    QString dealReceives(const QString &data);//粘包处理
    void appendReceiveData(const QString &data);
    void setReceiveData(const QString &data);
    const QString &getReceiveData()const;
private:
    class DataPrivate;
    DataPrivate *_p;
};

#endif // DEBUGGERDATAREUQIRE_H
