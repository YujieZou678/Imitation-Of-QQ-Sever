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

    void replyMessage(QTcpSocket*, const QByteArray &);  //回复消息
    QByteArray toJson_Message(const QString&);  //1个字符串转json格式发送

public slots:
    void onNewConnection();

private:
    enum class Purpose {  //枚举(class内部)
        CheckAccountNumber,
        Register
    };

    QTcpServer *server;
    QMap<QString, QTcpSocket*> map_Socket;  //创建多个嵌套字以支持多个用户同时在线
    QMap<QString, enum Purpose> map_Switch;  //需要初始化
};

#endif // TCPSERVER_H
