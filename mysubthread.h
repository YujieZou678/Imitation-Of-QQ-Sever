/*
function: 线程池调用的。用于子线程启用线程池处理请求。
author: zouyujie
date: 2024.3.25
*/
#ifndef MYSUBTHREAD_H
#define MYSUBTHREAD_H

#include <QObject>
#include <QRunnable>
#include <QTcpSocket>

class MySubThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    MySubThread(qintptr socketDescriptor, QObject *parent = nullptr);

    void run() override;  //重写，子线程执行
    void replyMessage(QTcpSocket*, const QByteArray &);  //回复消息
    QByteArray toJson_Message(const QString&);  //1个字符串转json格式发送

private:
    enum class Purpose {  //枚举(class内部)
        CheckAccountNumber,
        Register
    };

    qintptr socketDescriptor;  //获取当前socket描述符
    QMap<QString, enum Purpose> map_Switch;  //用于寻找信息是哪个目的
};

#endif // MYSUBTHREAD_H
