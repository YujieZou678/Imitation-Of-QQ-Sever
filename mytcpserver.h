/*
function: 服务端：主线程。
author: zouyujie
date: 2024.3.18
*/
#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

class MyThread;

class MyTcpServer : public QTcpServer
{
    Q_OBJECT

public:
    MyTcpServer(QTcpServer *parent = nullptr);
    ~MyTcpServer();

    void incomingConnection(qintptr socketDescriptor) override;  //重写, 分配socket给子线程
    void sendSignalAddByNum(int num, qintptr socketDescriptor);  //发送增加socket的信号
    void sendSignalPrintByNum(int num);                          //发送打印子线程启用的信号

signals:
    void toAllSubThread_Disconnect();  //断开与客户端连接

    void toThread1_PrintThreadStart();
    void toThread2_PrintThreadStart();
    void toThread3_PrintThreadStart();

    void toThread1_addOneSocket(qintptr socketDescriptor);
    void toThread2_addOneSocket(qintptr socketDescriptor);
    void toThread3_addOneSocket(qintptr socketDescriptor);

public slots:
    void onCloseListen();  //关闭监听
    void onOpenLisen();    //打开监听

private:
    QVector<QThread*> threads;
    QVector<MyThread*> myThreads;  //子线程们
};

#endif // TCPSERVER_H
