/*
function: 服务端：主线程。
author: zouyujie
date: 2024.3.18
*/
#include <QHostAddress>
#include <QThread>

#include "mytcpserver.h"
#include "mythread.h"
#include "myconfig.h"
#include "mydatabase.h"

MyTcpServer::MyTcpServer(QTcpServer *parent)
    : QTcpServer(parent)
{
    //server配置
    if (this->listen(QHostAddress(SEVER_LISTEN_ADDRESS), SEVER_LISTEN_PORT)) {
        qDebug() << "主线程" << QThread::currentThread() << ":"
                 << "Server正在监听"
                 << this->serverAddress()
                 << this->serverPort();
    }

    //初始化3个子线程, 以及移入的类
    for (int i=0; i<3; i++) {
        QThread *thread = new QThread;
        MyThread *myThread = new MyThread;
        threads.push_back(thread);
        myThreads.push_back(myThread);
        myThread->ID = myThreads.size();  //属于第几个线程
        myThread->moveToThread(thread);   //绑定
    }

    /* connect 主线程与子线程 && 子线程与子线程 */
    for (int i=0; i<3; i++) {
        /* 子 ---> 主 */
        connect(myThreads[i], &MyThread::needQuitThread, this, [=](){
            threads[i]->quit();
            threads[i]->wait();
            qDebug() << "主线程" << QThread::currentThread() << ":"
                     <<"子线程"+QString::number(myThreads[i]->ID)+"已关闭。";
        });
        connect(myThreads[i], &MyThread::needCloseListen, this, &MyTcpServer::onCloseListen);
        connect(myThreads[i], &MyThread::needOpenListen, this, &MyTcpServer::onOpenLisen);
        /* 主 ---> 子 */
        switch (myThreads[i]->ID) {
        case 1:
            /* 子线程打印启动信息 */
            connect(this, &MyTcpServer::toThread1_PrintThreadStart, myThreads[i], &MyThread::onPrintThreadStart);
            /* 子线程添加socket管理 */
            connect(this, &MyTcpServer::toThread1_addOneSocket, myThreads[i], &MyThread::addOneSocket);
            /* 子线程1-->子线程2 */
            connect(myThreads[i], &MyThread::toSubThread2_SendMsg, myThreads[i+1], &MyThread::onReceiveFromSubThread);
            /* 子线程1-->子线程3 */
            connect(myThreads[i], &MyThread::toSubThread3_SendMsg, myThreads[i+2], &MyThread::onReceiveFromSubThread);
            break;
        case 2:
            connect(this, &MyTcpServer::toThread2_PrintThreadStart, myThreads[i], &MyThread::onPrintThreadStart);
            connect(this, &MyTcpServer::toThread2_addOneSocket, myThreads[i], &MyThread::addOneSocket);
            /* 子线程2-->子线程1 */
            connect(myThreads[i], &MyThread::toSubThread1_SendMsg, myThreads[i-1], &MyThread::onReceiveFromSubThread);
            /* 子线程2-->子线程3 */
            connect(myThreads[i], &MyThread::toSubThread3_SendMsg, myThreads[i+1], &MyThread::onReceiveFromSubThread);
            break;
        case 3:
            connect(this, &MyTcpServer::toThread3_PrintThreadStart, myThreads[i], &MyThread::onPrintThreadStart);
            connect(this, &MyTcpServer::toThread3_addOneSocket, myThreads[i], &MyThread::addOneSocket);
            /* 子线程3-->子线程1 */
            connect(myThreads[i], &MyThread::toSubThread1_SendMsg, myThreads[i-2], &MyThread::onReceiveFromSubThread);
            /* 子线程3-->子线程2 */
            connect(myThreads[i], &MyThread::toSubThread2_SendMsg, myThreads[i-1], &MyThread::onReceiveFromSubThread);
            break;
        default:
            break;
        }
    }

    //数据库的初始化：创建User表, 只在运行时执行一次
    MyDatabase mydatabase;
    mydatabase.createUserTable();
}

MyTcpServer::~MyTcpServer()
{
    qDebug() << "主线程" << QThread::currentThread() << ":"
             << "主线程析构";

    for (auto myThread : myThreads) {  //释放myThreads
        if (myThread->socketCount == 0) {
            delete myThread;
            continue;
        }
        myThread->deleteLater();  //不能立刻删除！原因：直接删除是主线程去执行，而主线程不能操作子线程的socket
    }

    for (auto thread : threads) {  //终止所有子线程并释放threads
        if (thread->isRunning()) {
            thread->quit();
            thread->wait();
        }
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
    default:
        break;
    }
}

void MyTcpServer::onCloseListen()
{
    if (!this->isListening()) return;  //已经没有监听则返回
    this->close();  //可能有延迟，不能及时响应
    qDebug() << "主线程" << QThread::currentThread() << ":"
             << "Server已达最大连接数，暂时关闭。";
}

void MyTcpServer::onOpenLisen()
{
    if (this->isListening()) return;  //已经正在监听则返回
    if (this->listen(QHostAddress(SEVER_LISTEN_ADDRESS), SEVER_LISTEN_PORT)) {
        qDebug() << "主线程" << QThread::currentThread() << ":"
                 << "Server重新监听"
                 << this->serverAddress()
                 << this->serverPort();
    }
}

