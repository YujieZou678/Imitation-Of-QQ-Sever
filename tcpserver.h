/*
function: 仿QQ服务端。
author: zouyujie
date: 2024.3.18
*/
#ifndef TCPSERVER_H
#define TCPSERVER_H

class QTcpServer;
class QTcpSocket;

#include <QObject>

class TcpServer : public QObject
{
    Q_OBJECT

public:
    TcpServer(QObject *parent = nullptr);
    ~TcpServer();

public slots:
    void onNewConnection();

private:
    QTcpServer *server;
    QTcpSocket *socket;  //创建多个嵌套字以支持多个用户同时在线
};

#endif // TCPSERVER_H
