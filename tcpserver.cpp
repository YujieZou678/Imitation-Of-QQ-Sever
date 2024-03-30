/*
function: 主线程。
author: zouyujie
date: 2024.3.18
*/
#include <QHostAddress>
#include <QThread>

#include "mytcpserver.h"
#include "mythread.h"
#include "myconfig.h"

MyTcpServer::MyTcpServer(QTcpServer *parent)
    : QTcpServer(parent)
{
    //server配置
    this->setMaxPendingConnections(SEVER_MAX_CONNECTION);  //设置服务端最大连接数
    if (this->listen(QHostAddress(SEVER_LISTEN_ADDRESS), SEVER_LISTEN_PORT)) {
        qDebug() << "主线程" << QThread::currentThread() << ":"
                 << "Server正在监听"
                 << this->serverAddress()
                 << this->serverPort();
    }

    //初始化5个子线程, 以及移入的类
    for (int i=0; i<5; i++) {
        QThread *thread = new QThread;
        MyThread *myThread = new MyThread;
        threads.push_back(thread);
        myThreads.push_back(myThread);
        myThread->ID = myThreads.size();  //属于第几个线程
        myThread->moveToThread(thread);  //绑定
    }

    //connect 主线程与子线程 && 子线程与子线程
    for (int i=0; i<5; i++) {
        /* 子 ---> 主 */
        connect(myThreads[i], &MyThread::needQuitThread, this, [=](){
            threads[i]->quit();
            threads[i]->wait();
            qDebug() << "主线程" << QThread::currentThread() << ":"
                     <<"子线程"+QString::number(myThreads[i]->ID)+"已关闭。";
        });
        /* 主 ---> 子 */
        switch (myThreads[i]->ID) {
        case 1:
            connect(this, &MyTcpServer::toThread1_PrintThreadStart, myThreads[i], &MyThread::onPrintThreadStart);  //子线程打印启动信息
            connect(this, &MyTcpServer::toThread1_addOneSocket, myThreads[i], &MyThread::addOneSocket);  //子线程添加socket管理
            connect(myThreads[i], &MyThread::toThread2_SendMsg, myThreads[i+1], &MyThread::onReceiveFromSubThread);  //子线程1-->子线程2
            break;
        case 2:
            connect(this, &MyTcpServer::toThread2_PrintThreadStart, myThreads[i], &MyThread::onPrintThreadStart);
            connect(this, &MyTcpServer::toThread2_addOneSocket, myThreads[i], &MyThread::addOneSocket);

            break;
        case 3:
            connect(this, &MyTcpServer::toThread3_PrintThreadStart, myThreads[i], &MyThread::onPrintThreadStart);
            connect(this, &MyTcpServer::toThread3_addOneSocket, myThreads[i], &MyThread::addOneSocket);

            break;
        case 4:
            connect(this, &MyTcpServer::toThread4_PrintThreadStart, myThreads[i], &MyThread::onPrintThreadStart);
            connect(this, &MyTcpServer::toThread4_addOneSocket, myThreads[i], &MyThread::addOneSocket);

            break;
        case 5:
            connect(this, &MyTcpServer::toThread5_PrintThreadStart, myThreads[i], &MyThread::onPrintThreadStart);
            connect(this, &MyTcpServer::toThread5_addOneSocket, myThreads[i], &MyThread::addOneSocket);

            break;
        default:
            break;
        }
    }
}

MyTcpServer::~MyTcpServer()
{
    qDebug() << "主线程" << QThread::currentThread() << ":"
             << "主线程析构";
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
    /*
     * 主线程分配socket给子线程管理;
     * 并找出最优选择 */

    int min = 10000;
    for (int i=0; i<myThreads.size(); i++) {
        if (myThreads[i]->socketCount == 0) {  //此时线程是关闭的
            int num = i+1;  //得到是哪一个线程
            threads[i]->start();  //启用线程
            sendSignalPrintByNum(num);  //发送信号启用子线程
            sendSignalAddByNum(num, socketDescriptor);  //发送信号增加socket
            return;
        }

        if (min > myThreads[i]->socketCount) { min = myThreads[i]->socketCount; }
    }
    for (int i=0; i<myThreads.size(); i++) {
        if (myThreads[i]->socketCount == min) {
            int num = i+1;  //得到是哪一个线程
            sendSignalAddByNum(num, socketDescriptor);  //发送信号增加socket
            break;
        }
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

void MyTcpServer::sendSignalPrintByNum(int num)
{
    switch (num) {
    case 1:
        emit toThread1_PrintThreadStart();
        break;
    case 2:
        emit toThread2_PrintThreadStart();
        break;
    case 3:
        emit toThread3_PrintThreadStart();
        break;
    case 4:
        emit toThread4_PrintThreadStart();
        break;
    case 5:
        emit toThread5_PrintThreadStart();
        break;
    default:
        break;
    }
}

