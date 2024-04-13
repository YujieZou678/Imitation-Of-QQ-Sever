/*
function: 线程池调用的。用于子线程启用线程池处理请求。
author: zouyujie
date: 2024.3.25
*/
#include <QThread>

#include "mysubthread.h"
#include "mydatabase.h"

MySubThread::MySubThread(MySocket *_socket, QJsonDocument doc, QObject *parent) :
    QObject(parent)
{
    //map_Switch
    map_Switch = {
        {"CheckAccountNumber", Purpose::CheckAccountNumber},
        {"Register", Purpose::Register},
        {"Login", Purpose::Login},
        {"SingleChat", Purpose::SingleChat}
    };

    //socket
    socket = _socket;

    //myPurpose
    QString data_Purpose = doc["Purpose"].toString();
    myPurpose = map_Switch[data_Purpose];  //初始化myPurpose

    //根据对应目标给初始值赋值
    switch (myPurpose) {
    case Purpose::CheckAccountNumber: {
        accountNumber = doc["AccountNumber"].toString();
        break;
    }
    case Purpose::Register: {
        accountNumber = doc["AccountNumber"].toString();
        password = doc["Password"].toString();
        break;
    }
    case Purpose::Login: {
        accountNumber = doc["AccountNumber"].toString();
        password = doc["Password"].toString();
        break;
    }
    default:
        break;
    }
}

void MySubThread::run()
{
    switch (myPurpose) {
    case Purpose::CheckAccountNumber: {
        MyDatabase myDatabase;
        QString isExit = myDatabase.checkAccountNumber(accountNumber);
        emit finished_CheckAccountNumber(socket, isExit);
        qDebug() << "线程池" << QThread::currentThread() << ":"
                 << "账号检测完毕。";
        break;
    }
    case Purpose::Register: {
        MyDatabase myDatabase;
        myDatabase.addUser(accountNumber, password);
        qDebug() << "线程池" << QThread::currentThread() << ":"
                 << "用户注册完毕。";
        break;
    }
    case Purpose::Login: {
        MyDatabase myDatabase;
        QString isRight = myDatabase.checkLogin(accountNumber, password);
        emit finished_Login(socket, isRight);
        qDebug() << "线程池" << QThread::currentThread() << ":"
                 << "登陆检测完毕。";
        break;
    }
    default:
        break;
    }
}
