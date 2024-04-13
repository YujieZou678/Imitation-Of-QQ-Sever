#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <QTcpSocket>

class MySocket : public QTcpSocket
{
public:
    MySocket(QTcpSocket *parent = nullptr);
    ~MySocket();

    bool ifNeedReceiveFile{false};  //是否需要接收文件
    QByteArray file;  //文件数据
    QString ID;  //qq号
    qint64 fileSize{0};  //文件大小
    qint64 receiveSize{0};  //已接收大小
    int count{0};  //接收次数
};

#endif // MYSOCKET_H
