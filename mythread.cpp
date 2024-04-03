/*
function: 子线程。用于管理并监听主线程分下来socket。
author: zouyujie
date: 2024.3.27
*/
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QThreadPool>

#include "mythread.h"
#include "mysubthread.h"
#include "myconfig.h"

MyThread::MyThread(QObject *parent) :
    QObject(parent)
{
    //map_Switch
    map_Switch = {
        {"CheckAccountNumber", Purpose::CheckAccountNumber},
        {"Register", Purpose::Register},
        {"SingleChat", Purpose::SingleChat}
    };

    //myThreadPool
    myThreadPool = QThreadPool::globalInstance();
    myThreadPool->setMaxThreadCount(10);  //线程池最大线程数
}

MyThread::~MyThread()
{
    qDebug() << "主线程" << QThread::currentThread() << ":"
             <<"子线程"+QString::number(ID)+"析构";

    QList<QTcpSocket*> sockets = socketsMap.values();
    for (auto socket : sockets) {
        delete socket;
        socket = nullptr;
    }
}

void MyThread::addOneSocket(qintptr socketDescriptor)
{
    socketCount += 1;  //数量加1
    if (socketCount > SEVER_MAX_CONNECTION/3) emit needCloseListen();

    QTcpSocket *socket = new QTcpSocket;
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "socket->setSocketDescriptor(socketDescriptor) failed!";
        return;
    }
    QString ip_port = getIp_Port(socket);
    qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
             << ip_port+" 已连接至服务器。";
    socketsMap.insert(ip_port, socket);

    //connect
    connect(socket, &QTcpSocket::readyRead, [=](){
        /* 处理请求 */
        if (socket->bytesAvailable() <= 0) return;  //字节为空则退出
        QByteArray data = socket->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QString data_Purpose = doc["Purpose"].toString();
        enum Purpose purpose = map_Switch[data_Purpose];
        switch (purpose) {
        case Purpose::CheckAccountNumber: {
            /* 线程池调用数据库检验 */
            MySubThread *mySubThread = new MySubThread(socket, doc);
            connect(mySubThread, &MySubThread::finished_CheckAccountNumber, this, &MyThread::onFinished_CheckAccountNumber);
            myThreadPool->start(mySubThread);  //线程池会自动释放mySubThread
            break;
        }
        case Purpose::Register: {
            MySubThread *mySubThread = new MySubThread(socket, doc);
            myThreadPool->start(mySubThread);
            break;
        }
        case Purpose::SingleChat: {
            QString object = doc["Object"].toString();    //对象
            QString content = doc["Content"].toString();  //内容
            if (object == "572211") {
                qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
                         << "已发送给"+object;
                    emit toThread2_SendMsg(content);
            }
            break;
        }
        default:
            break;
        }
    });
    connect(socket, &QTcpSocket::disconnected, [=](){
        socketsMap.remove(ip_port);
        socket->deleteLater();  //不能立刻删除!!!

        socketCount -= 1;
        if (socketCount == 0) emit needQuitThread();
        if (socketCount < SEVER_MAX_CONNECTION/3) emit needOpenListen();
    });
}

QByteArray MyThread::info_SendMsg(const QString &msg)
{
    QJsonObject json;
    json.insert("Reply", msg);  //目的
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    return data;
}

QString MyThread::getIp_Port(QTcpSocket *socket)
{
    QString ip = socket->peerAddress().toString();
    QString port = QString::number(socket->peerPort());
    QString ip_port = ip+":"+port;

    return ip_port;
}

void MyThread::onPrintThreadStart()
{
    qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
             << "一个子线程已启用。";
}

void MyThread::onReceiveFromSubThread(const QString &msg)
{
    qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
             << msg;
}

void MyThread::onFinished_CheckAccountNumber(QTcpSocket *socket, const QString &isExit)
{
    socket->write(info_SendMsg(isExit));  //发送存在的信息
}
