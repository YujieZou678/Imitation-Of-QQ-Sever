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
#include <QVector>
#include <QMap>

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
    QMap<QString, QTcpSocket*> map_Socket;  //创建多个嵌套字以支持多个用户同时在线
};

#endif // TCPSERVER_H
