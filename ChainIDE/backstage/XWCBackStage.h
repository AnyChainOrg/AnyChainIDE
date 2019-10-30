#ifndef XWCBACKSTAGE_H
#define XWCBACKSTAGE_H

#include "BackStageBase.h"
class XWCBackStage : public BackStageBase
{
public:
    explicit XWCBackStage(int type = 1,QObject *parent = nullptr);//链类型1==测试 2==正式
    ~XWCBackStage()override final;

public:
    void startExe(const QString &appDataPath = "")override final;
    bool exeRunning()override final;
    QProcess *getProcess()const override final;
    void ReadyClose()override final;
private:
    void startNodeProc();
    void startClientProc();
private slots:
    void readNodeStandError();
    void readNodeStandOutput();
    void readClientStandError();
    void readClientStandOutput();
private:
    void checkNodeMessage(const QString &message);
    void checkClientMessage(const QString &message);

    void onNodeExeStateChanged();
    void onClientExeStateChanged();
public slots:
    void rpcPostedSlot(const QString &,const QString &)override final;
protected slots:
    void rpcReceivedSlot(const QString &id,const QString &message)override final;
private:
    void initSocketManager();
private:
    class DataPrivate;
    DataPrivate *_p;
};

#endif // XWCBACKSTAGE_H
