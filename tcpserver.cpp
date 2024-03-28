/*
function: 主线程。
author: zouyujie
date: 2024.3.18
*/
#include <QHostAddress>
#include <QThread>

#include "mytcpserver.h"

MyTcpServer::MyTcpServer(QTcpServer *parent)
    : QTcpServer(parent)
{
    //server配置
    this->setMaxPendingConnections(5);  //设置服务端最大连接数
    if (this->listen(QHostAddress::LocalHost, 2222)) {
        qDebug() << "主线程" << QThread::currentThread() << ":"
                 << "Server正在监听"
                 << this->serverAddress()
                 << this->serverPort();
    }
}

MyTcpServer::~MyTcpServer()
{
    qDebug() << "主线程" << QThread::currentThread() << ":"
             << "MyTcpServer析构";
    for (auto myThread : myThreads) {  //释放myThreads
        delete myThread;
        myThread = nullptr;
    }

    for (auto thread : threads) {  //终止所有子线程并释放threads
        thread->quit();
        thread->wait();
        delete thread;
        thread = nullptr;
    }
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    /* 主线程分配socket给子线程管理 */
    if (myThreads.size() < 5) {  //允许子线程最大数量
        addOneSubThread();
        int num = myThreads.size();  //得到是哪个线程
        sendSignalAddByNum(num, socketDescriptor);  //发送信号
    }
    else {
        //找出最优选择
        int min = 10000;
        for (int i=0; i<myThreads.size(); i++) {
            if (myThreads[i]->socketCount == 0) {  //此时线程是关闭的
                threads[i]->start();  //重新启用线程
                int num = i+1;  //得到是哪一个线程
                qDebug() << "主线程" << QThread::currentThread() << ":"
                         << "子线程"+QString::number(num) << "重新启用";
                sendSignalAddByNum(num, socketDescriptor);  //发送信号
                return;
            }

            if (min > myThreads[i]->socketCount) { min = myThreads[i]->socketCount; }
        }
        for (int i=0; i<myThreads.size(); i++) {
            if (myThreads[i]->socketCount == min) {
                int num = i+1;
                sendSignalAddByNum(num, socketDescriptor);  //发送信号
                break;
            }
        }
    }
}

void MyTcpServer::addOneSubThread()
{
    //添加一个子线程
    QThread *thread = new QThread;
    MyThread *myThread = new MyThread;
    threads.push_back(thread);
    myThreads.push_back(myThread);
    myThread->ID = myThreads.size();  //属于第几个线程

    myThread->moveToThread(thread);  //绑定
    thread->start();

    //connect
    connect(myThread, &MyThread::needQuitThread, this, [=](){
        thread->quit();
        thread->wait();
        qDebug() << "主线程" << QThread::currentThread() << ":"
                 <<"子线程"+QString::number(myThread->ID)+"已关闭。";
    });
    switch (myThread->ID) {
    case 1:
        connect(this, &MyTcpServer::toThread1_PrintThreadStart, myThread, &MyThread::onPrintThreadStart);  //子线程打印启动信息
        connect(this, &MyTcpServer::toThread1_addOneSocket, myThread, &MyThread::addOneSocket);  //子线程添加socket管理

        emit toThread1_PrintThreadStart();
        break;
    case 2:
        connect(this, &MyTcpServer::toThread2_PrintThreadStart, myThread, &MyThread::onPrintThreadStart);
        connect(this, &MyTcpServer::toThread2_addOneSocket, myThread, &MyThread::addOneSocket);

        emit toThread2_PrintThreadStart();
        break;
    case 3:
        connect(this, &MyTcpServer::toThread3_PrintThreadStart, myThread, &MyThread::onPrintThreadStart);
        connect(this, &MyTcpServer::toThread3_addOneSocket, myThread, &MyThread::addOneSocket);

        emit toThread3_PrintThreadStart();
        break;
    case 4:
        connect(this, &MyTcpServer::toThread4_PrintThreadStart, myThread, &MyThread::onPrintThreadStart);
        connect(this, &MyTcpServer::toThread4_addOneSocket, myThread, &MyThread::addOneSocket);

        emit toThread4_PrintThreadStart();
        break;
    case 5:
        connect(this, &MyTcpServer::toThread5_PrintThreadStart, myThread, &MyThread::onPrintThreadStart);
        connect(this, &MyTcpServer::toThread5_addOneSocket, myThread, &MyThread::addOneSocket);

        emit toThread5_PrintThreadStart();
        break;
    default:
        break;
    }
}

void MyTcpServer::sendSignalAddByNum(int num, qintptr socketDescriptor)
{
    switch (num) {
    case 1:
        emit toThread1_addOneSocket(socketDescriptor);
        break;
    case 2:
        emit toThread2_addOneSocket(socketDescriptor);
        break;
    case 3:
        emit toThread3_addOneSocket(socketDescriptor);
        break;
    case 4:
        emit toThread4_addOneSocket(socketDescriptor);
        break;
    case 5:
        emit toThread5_addOneSocket(socketDescriptor);
        break;
    default:
        break;
    }
}

