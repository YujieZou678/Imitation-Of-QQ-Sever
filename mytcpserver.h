/*
function: 主线程。
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

    void incomingConnection(qintptr socketDescriptor) override;  //重写, 获得socket
    void sendSignalAddByNum(int num, qintptr socketDescriptor);  //发送增加socket的信号
    void sendSignalPrintByNum(int num);  //发送打印子线程启用的信号

signals:
    void toThread1_PrintThreadStart();
    void toThread2_PrintThreadStart();
    void toThread3_PrintThreadStart();
    void toThread4_PrintThreadStart();
    void toThread5_PrintThreadStart();

    void toThread1_addOneSocket(qintptr socketDescriptor);
    void toThread2_addOneSocket(qintptr socketDescriptor);
    void toThread3_addOneSocket(qintptr socketDescriptor);
    void toThread4_addOneSocket(qintptr socketDescriptor);
    void toThread5_addOneSocket(qintptr socketDescriptor);

private:
    QVector<QThread*> threads;     //容器管理子线程
    QVector<MyThread*> myThreads;  //移入对应子线程的类
};

#endif // TCPSERVER_H
