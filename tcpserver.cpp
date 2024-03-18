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
    //初始化服务
    socket = new QTcpSocket(this);
    server = new QTcpServer(this);

    server->setMaxPendingConnections(1);  //设置服务端最大连接数
    if (server->listen(QHostAddress::LocalHost, 2222)) {
        qDebug() << "Server正在监听" << server->serverAddress() << server->serverPort();
    }

    connect(server, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);  //服务端如果有新的连接
}

TcpServer::~TcpServer()
{
    socket->disconnectFromHost();  //断开与客户端的连接
}

void TcpServer::onNewConnection()
{
    while (server->hasPendingConnections()) {
        socket = server->nextPendingConnection();  //一个连接对应一个嵌套字
        qDebug() << socket->peerAddress() << socket->peerPort() << "已连接";

        //如果有新的消息
        connect(socket, &QTcpSocket::readyRead, this, [=](){
            if (socket->bytesAvailable() <= 0) return;
            QByteArray data = socket->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            qDebug() << "接收消息" << doc["accountNumber"].toString() << doc["passWord"].toString();
        });
        //如果客户端断开连接
        connect(socket, &QTcpSocket::disconnected, this, [=](){
            socket->deleteLater();
        });
    }
}

