/*
function: 线程池调用的。用于子线程启用线程池处理请求。
author: zouyujie
date: 2024.3.25
*/
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>

#include "mysubthread.h"
#include "mydatabase.h"

MySubThread::MySubThread(MySocket *_socket, QJsonDocument _doc, QObject *parent) :
    QObject(parent)
{
    //map_Switch
    map_Switch = {
        {"CheckAccountNumber", Purpose::CheckAccountNumber},
        {"Register", Purpose::Register},
        {"Login", Purpose::Login},
        {"CreateGroup", Purpose::CreateGroup}
    };

    //socket
    socket = _socket;

    //myPurpose
    QString data_Purpose = _doc["Purpose"].toString();
    myPurpose = map_Switch[data_Purpose];  //初始化myPurpose

    //根据对应目标给初始值赋值
    switch (myPurpose) {
    case Purpose::CheckAccountNumber: {
        accountNumber = _doc["AccountNumber"].toString();
        check = _doc["Check"].toString();
        break;
    }
    case Purpose::Register: {
        accountNumber = _doc["AccountNumber"].toString();
        password = _doc["Password"].toString();
        break;
    }
    case Purpose::Login: {
        accountNumber = _doc["AccountNumber"].toString();
        password = _doc["Password"].toString();
        break;
    }
    case Purpose::CreateGroup: {
        accountNumber = _doc["GroupNumber"].toString();
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
        QJsonObject json;
        json.insert("Check", check);
        json.insert("Reply", isExit);
        json.insert("AccountNumber", accountNumber);
        QJsonDocument _doc(json);
        emit finished_CheckAccountNumber(socket, _doc);
        qDebug() << "线程池" << QThread::currentThread() << ":"
                 << "账号检测完毕。";
        break;
    }
    case Purpose::Register: {
        MyDatabase myDatabase;
        QString isOk = myDatabase.addUser(accountNumber, password);
        emit finished_Register(socket, isOk);
        qDebug() << "线程池" << QThread::currentThread() << ":"
                 << "用户注册完毕。";
        break;
    }
    case Purpose::Login: {
        MyDatabase myDatabase;
        QString isRight = myDatabase.checkLogin(accountNumber, password);
        emit finished_Login(socket, isRight, accountNumber);
        qDebug() << "线程池" << QThread::currentThread() << ":"
                 << "登陆检测完毕。";
        break;
    }
    case Purpose::CreateGroup: {
        MyDatabase myDatabase;
        QString isExit = myDatabase.checkAccountNumber(accountNumber);
        QJsonObject json;
        json.insert("GroupNumber", accountNumber);
        json.insert("Reply", isExit);
        QJsonDocument _doc(json);
        emit finished_CheckGroupNumber(socket, _doc);
        qDebug() << "线程池" << QThread::currentThread() << ":"
                 << "群账号检测完毕。";
        break;
    }
    default:
        break;
    }
}
