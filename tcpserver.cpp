/*
function: 仿QQ服务端。
author: zouyujie
date: 2024.3.18
*/
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

#include <QJsonDocument>
#include <QJsonObject>

#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent)
    : QObject(parent)
{
    //server
    server = new QTcpServer(this);
    server->setMaxPendingConnections(1);  //设置服务端最大连接数
    if (server->listen(QHostAddress::LocalHost, 2222)) {
        qDebug() << "Server正在监听" << server->serverAddress() << server->serverPort();
    }
    connect(server, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);  //服务端如果有新的连接

    //map_Switch
    map_Switch = {
        {"CheckAccountNumber", Purpose::CheckAccountNumber},
        {"Register", Purpose::Register}
    };
//    enum Purpose temp = map_Switch["Register"];
//    switch (temp) {
//    case Purpose::CheckAccountNumber:
//        qDebug() << 123;
//        break;
//    case Purpose::Register:
//        qDebug() << "456";
//        break;
//    default:
//        break;
//    }
}

TcpServer::~TcpServer()
{
    //断开与所有客户端的连接
    QList<QTcpSocket*> values = map_Socket.values();
    for (auto socket : values) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
}

void TcpServer::replyMessage(QTcpSocket *socket, const QByteArray &data)
{
    socket->write(data);
}

QByteArray TcpServer::toJson_Message(const QString &msg)
{
    QJsonObject json;
    json.insert("Reply", msg);  //目的
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    return data;
}

void TcpServer::onNewConnection()
{
    while (server->hasPendingConnections()) {
        QTcpSocket *socket = server->nextPendingConnection();  //一个连接对应一个嵌套字
        QString ip = socket->localAddress().toString();
        QString port = QString::number(socket->localPort());
        map_Socket.insert(ip+port, socket);  //ip+port : socket

        qDebug() << socket->peerAddress() << socket->peerPort() << "已连接";

        //如果有新的消息
        connect(socket, &QTcpSocket::readyRead, this, [=](){
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
            map_Socket.remove(ip+port);
            socket->deleteLater();
        });
    }
}

