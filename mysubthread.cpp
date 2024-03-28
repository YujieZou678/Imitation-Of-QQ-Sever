/*
function: 线程池调用的。用于子线程启用线程池处理请求。
author: zouyujie
date: 2024.3.25
*/
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>

#include "mysubthread.h"

extern QTcpSocket *socket;

MySubThread::MySubThread(qintptr _socketDescriptor, QObject *parent) :
    QObject(parent)
{
    //socketDescriptor
    socketDescriptor = _socketDescriptor;

    //map_Switch
    map_Switch = {
        {"CheckAccountNumber", Purpose::CheckAccountNumber},
        {"Register", Purpose::Register}
    };
}

void MySubThread::run()
{
    qDebug() << "123";
    QTcpSocket *socket = new QTcpSocket;
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "socket->setSocketDescriptor(socketDescriptor) failed!";
        return;
    }

    //如果有新的消息
    connect(socket, &QTcpSocket::readyRead, this, [=](){
        qDebug() << "receive";
        /* 子线程处理消息 */
        if (socket->bytesAvailable() <= 0) return;  //字节为空则退出
        QByteArray data = socket->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QString data_Purpose = doc["purpose"].toString();
        enum Purpose purpose = map_Switch[data_Purpose];
        switch (purpose) {
        case Purpose::CheckAccountNumber:
            //QString accountNumber = doc["accountNumber"].toString();
            //调用数据库检验
            replyMessage(socket, toJson_Message("false"));  //发送存在的信息
            qDebug() << "返回false";
            break;
        case Purpose::Register:
            break;
        }
    });
    //如果客户端断开连接
    connect(socket, &QTcpSocket::disconnected, this, [=](){
        socket->deleteLater();
    });
}

void MySubThread::replyMessage(QTcpSocket *socket, const QByteArray &data)
{
    socket->write(data);
}

QByteArray MySubThread::toJson_Message(const QString &msg)
{
    QJsonObject json;
    json.insert("Reply", msg);  //目的
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    return data;
}
