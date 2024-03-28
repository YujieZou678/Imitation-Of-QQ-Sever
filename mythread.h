/*
function: 子线程。用于管理并监听主线程分下来socket。
author: zouyujie
date: 2024.3.27
*/
#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include <QTcpSocket>

class MyThread : public QObject
{
    Q_OBJECT

public:
    MyThread(QObject *parent = nullptr);
    ~MyThread();

    void addOneSocket(qintptr socketDescriptor);  //添加一个socket
    QByteArray toJson_Message(const QString&);  //1个字符串转json格式发送
    QString getIp_Port(QTcpSocket*);  //获得一个socket的ip_port

    int socketCount{0};   //当前正在管理的socket数量
    int ID;  //属于第几个线程

signals:
    void needQuitThread();  //当管理socket数量为0时需要关闭线程

public slots:
    void onPrintThreadStart();

private:
    QMap<QString,QTcpSocket*> socketsMap;
};

#endif // MYTHREAD_H
