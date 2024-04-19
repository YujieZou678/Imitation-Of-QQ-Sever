/*
function: 子线程。用于管理并监听主线程分下来socket。
author: zouyujie
date: 2024.3.27
*/
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
        {"ReceiveFile", Purpose::ReceiveFile},
        {"ChangePersonalData", Purpose::ChangePersonalData},
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
                saveProfileImage(socket->accountNumber, socket->file, socket->fileSize);  //保存图像文件

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
            socket->accountNumber = doc["AccountNumber"].toString();
            socket->file.clear();
            socket->receiveSize = 0;
            socket->count = 0;
            socket->ifNeedReceiveFile = true;  //该socket下个传输为文件

            onFinished_PrepareSendFile(socket);  //回应准备接收文件
            break;
        }
        case Purpose::ReceiveFile: {
            QString reply = doc["Reply"].toString();  //得到答复
            if (reply == "true") {
                /* 开始发送文件 */
                qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
                         << "开始发送文件";
                QString accountNumber = doc["AccountNumber"].toString();
                QByteArray data = getProfileImageData(accountNumber);
                startSendFile(socket, data);
            }
            break;
        }
        case Purpose::ChangePersonalData: {
            /* 更新个人信息 */
            savePersonalData(doc);
            qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
                     << "个人信息更新完成";
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

void MyThread::saveProfileImage(const QString &accountNumber, const QByteArray &data, qintptr size)
{
    settings->setValue(accountNumber+"/ProfileImage/Size", size);  //存入大小
    if (!data.isEmpty()) settings->setValue(accountNumber+"/ProfileImage/Data", data);  //存入图像数据
}

QByteArray MyThread::getProfileImageData(const QString &accountNumber)
{
    settings->beginGroup(accountNumber);   //进入目录
    settings->beginGroup("ProfileImage");  //进入目录
    QByteArray data = settings->value("Data").toByteArray();  //获取文件数据
    settings->endGroup();  //退出目录
    settings->endGroup();  //退出目录

    return data;
}

int MyThread::getProfileImageSize(const QString &accountNumber)
{
    settings->beginGroup(accountNumber);  //进入目录
    settings->beginGroup("ProfileImage");  //进入目录
    int size = settings->value("Size").toInt();  //获取文件数据大小
    settings->endGroup();  //退出目录
    settings->endGroup();  //退出目录

    return size;
}

void MyThread::startSendFile(MySocket *socket, const QByteArray &_data)
{
    qintptr oneSend_Size = 4000000;  //一次最大传输:四百万
    if (_data.size() < oneSend_Size) {  //一次性传输
        qintptr fileSize = socket->write(_data);
        socket->flush();  //立刻传输
        qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
                 << "图像文件发送完毕 大小："+QString::number(fileSize);
    }
    else {  //多次传输
        qintptr hadSend_Size = 0;  //已经传输的大小
        for (int i=0; i<1000; i++) {
            if (hadSend_Size >= _data.size()) {  //检测是否发送完毕
                qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
                         << "图像文件发送完毕 大小："+QString::number(hadSend_Size);
                break;
            }

            QByteArray data;
            if (_data.size()-hadSend_Size < oneSend_Size) {  //最后一次字节不够
                data = _data.last(_data.size()-hadSend_Size);
            } else {
                data = _data.sliced(hadSend_Size, oneSend_Size);
            }
            qintptr oneWrite_Size = socket->write(data);
            socket->flush();  //立刻传输
            qDebug() << "子线程"+QString::number(ID) << QThread::currentThread() << ":"
                     << "图像文件第"+QString::number(i+1)+"次发送 大小："+QString::number(oneWrite_Size);
                            hadSend_Size += oneWrite_Size;
        }
    }
}

void MyThread::savePersonalData(const QJsonDocument &doc)
{
    QString accountNumber = doc["AccountNumber"].toString();
    /* 按账号存储各种信息 */
    settings->setValue(accountNumber+"/PersonalData/NickName", doc["NickName"].toString());  //昵称
    settings->setValue(accountNumber+"/PersonalData/Sex", doc["Sex"].toString());            //性别
    settings->setValue(accountNumber+"/PersonalData/ZodiacSign", doc["ZodiacSign"].toString());//属相
    settings->setValue(accountNumber+"/PersonalData/BloodGroup", doc["BloodGroup"].toString());//血型
    settings->setValue(accountNumber+"/PersonalData/PersonalSignature", doc["PersonalSignature"].toString());//个性签名
}

QString MyThread::getPersonalData(const QString &accountNumber, const QString &key)
{
    settings->beginGroup(accountNumber);   //进入目录
    settings->beginGroup("PersonalData");  //进入目录
    QString data = settings->value(key).toString();  //获取个人信息key对应的值
    settings->endGroup();  //退出目录
    settings->endGroup();  //退出目录

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

void MyThread::onFinished_CheckAccountNumber(MySocket *socket, const QJsonDocument &_doc)
{
    QString check = _doc["Check"].toString();
    QString isExit = _doc["Reply"].toString();

    QJsonObject json;
    json.insert("Purpose", "CheckAccountNumber");  //目的
    json.insert("Check", check);
    json.insert("Reply", isExit);  //回复

    if (check == "Login") {  //用于登陆：个人信息 头像文件
        QString accountNumber = _doc["AccountNumber"].toString();       //账号
        qint64 fileSize = getProfileImageSize(accountNumber);           //文件大小
        QString NickName = getPersonalData(accountNumber, "NickName");  //昵称
        QString Sex = getPersonalData(accountNumber, "Sex");            //性别
        QString ZodiacSign = getPersonalData(accountNumber, "ZodiacSign");//属相
        QString BloodGroup = getPersonalData(accountNumber, "BloodGroup");//血型
        QString PersonalSignature = getPersonalData(accountNumber, "PersonalSignature");//个性签名

        json.insert("AccountNumber", accountNumber);
        json.insert("FileSize", fileSize);
        json.insert("NickName", NickName);
        json.insert("Sex", Sex);
        json.insert("ZodiacSign", ZodiacSign);
        json.insert("BloodGroup", BloodGroup);
        json.insert("PersonalSignature", PersonalSignature);
    }

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
