/*
function: 子线程。用于管理并监听主线程分下来socket。
author: zouyujie
date: 2024.3.27
*/
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QThreadPool>
#include <QImage>
#include <QSettings>

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
        {"Login", Purpose::Login},
        {"PrepareSendFile", Purpose::PrepareSendFile},
        {"SingleChat", Purpose::SingleChat}
    };

    //myThreadPool
    myThreadPool = QThreadPool::globalInstance();
    myThreadPool->setMaxThreadCount(10);  //线程池最大线程数

    //settings
    settings = new QSettings("config/local.ini", QSettings::NativeFormat, this);
}

MyThread::~MyThread()
{
    qDebug() << "主线程" << QThread::currentThread() << ":"
             <<"子线程"+QString::number(ID)+"析构";

    QList<MySocket*> sockets = socketsMap.values();
    for (auto socket : sockets) {
        delete socket;
        socket = nullptr;
    }
}

void MyThread::addOneSocket(qintptr socketDescriptor)
{
    socketCount += 1;  //数量加1
    if (socketCount > SEVER_MAX_CONNECTION/3) emit needCloseListen();

    MySocket *socket = new MySocket;
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "socket->setSocketDescriptor(socketDescriptor) failed!";
        return;
    }
    QString ip_port = getIp_Port(socket);
    qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
             << ip_port+" 已连接至服务器。";
    socketsMap.insert(ip_port, socket);

    //connect
    connect(socket, &MySocket::readyRead, [=](){
        if (socket->ifNeedReceiveFile) {
            /* 开始接收头像文件 */
            socket->count += 1;
            //QByteArray data = QByteArray::fromBase64(socket->readAll());
            QByteArray data = socket->readAll();
            socket->file.append(data);
            socket->receiveSize += data.size();
            qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
                     << "开始接收图像文件 次数"+QString::number(socket->count)+" "+QString::number(socket->receiveSize);

            if (socket->fileSize <= socket->receiveSize) {  //图像文件接收完毕
                QJsonObject json;
                json.insert("ID", socket->ID);
                QJsonDocument doc(json);
                savePersonlInfo(doc, socket->file);  //保存个人信息

                socket->ifNeedReceiveFile = false;
                onFinished_SendFile(socket);  //回应接收完毕
                qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
                         << "图像文件接收完毕。";
            }
            return;
        }

        /* 处理请求 */
        if (socket->bytesAvailable() <= 0) return;  //字节为空则退出
        QByteArray data = socket->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QString data_Purpose = doc["Purpose"].toString();
        if (data_Purpose == "") { qDebug() << "传输文件过大，没能一次性传输！"; return; }
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
        case Purpose::Login: {
            MySubThread *mySubThread = new MySubThread(socket, doc);
            connect(mySubThread, &MySubThread::finished_Login, this, &MyThread::onFinished_Login);
            myThreadPool->start(mySubThread);
            break;
        }
        case Purpose::PrepareSendFile: {
            /* 初始化文件数据 */
            socket->fileSize = doc["FileSize"].toInt();
            qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
                     << "准备接收图像文件 大小："+QString::number(socket->fileSize);
            socket->ID = doc["ID"].toString();
            socket->file.clear();
            socket->receiveSize = 0;
            socket->count = 0;
            socket->ifNeedReceiveFile = true;  //该socket下个传输为文件

            onFinished_PrepareSendFile(socket);  //回应准备接收文件
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
    connect(socket, &MySocket::disconnected, [=](){
        socketsMap.remove(ip_port);
        socket->deleteLater();  //不能立刻删除!!!

        socketCount -= 1;
        if (socketCount == 0) emit needQuitThread();
        if (socketCount < SEVER_MAX_CONNECTION/3) emit needOpenListen();
    });
}

QString MyThread::getIp_Port(MySocket *socket)
{
    QString ip = socket->peerAddress().toString();
    QString port = QString::number(socket->peerPort());
    QString ip_port = ip+":"+port;

    return ip_port;
}

void MyThread::savePersonlInfo(const QJsonDocument &doc, const QByteArray &data)
{
    QString ID = doc["ID"].toString();  //qq号
    if (!data.isEmpty()) settings->setValue(ID+"/ProfileImage", data);
}

QByteArray MyThread::getProfileImage()
{
    settings->beginGroup("2894841947");  //进入目录
    QByteArray data = settings->value("ProfileImage").toByteArray();
    settings->endGroup();  //退出目录

//    QImage image;
//    image.loadFromData(data, "PNG");
//    image.save("/root/my_test/Server/test.png", "PNG");  //保存该文件

    return data;
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

void MyThread::onFinished_CheckAccountNumber(MySocket *socket, const QString &isExit)
{
    QJsonObject json;
    json.insert("Purpose", "CheckAccountNumber");  //目的
    json.insert("Reply", isExit);  //回复
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    socket->write(data);  //发送存在的信息
}

void MyThread::onFinished_Login(MySocket *socket, const QString &isRight)
{
    QJsonObject json;
    json.insert("Purpose", "Login");  //目的
    json.insert("Reply", isRight);  //回复
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    socket->write(data);  //发送存在的信息
}

void MyThread::onFinished_PrepareSendFile(MySocket *socket)
{
    QJsonObject json;
    json.insert("Purpose", "PrepareSendFile");  //目的
    json.insert("Reply", "true");  //回复
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    socket->write(data);  //发送存在的信息
}

void MyThread::onFinished_SendFile(MySocket *socket)
{
    QJsonObject json;
    json.insert("Purpose", "SendFile");  //目的
    json.insert("Reply", "true");  //回复
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    socket->write(data);  //发送存在的信息
}
