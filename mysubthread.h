/*
function: 线程池调用的。用于子线程启用线程池处理请求。
author: zouyujie
date: 2024.3.25
*/
#ifndef MYSUBTHREAD_H
#define MYSUBTHREAD_H

#include <QObject>
#include <QRunnable>
#include <QJsonDocument>
#include <QMap>
#include <QTcpSocket>

class MySubThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    MySubThread(QTcpSocket *, QJsonDocument doc, QObject *parent = nullptr);
    void run() override;  //重写，子线程执行

signals:
    void finished_CheckAccountNumber(QTcpSocket *, QString);  //信号：账号检测完毕
    void finished_Login(QTcpSocket *, QString);  //信号：登陆检测完毕

private:
    enum class Purpose {  //枚举(class内部使用)
        CheckAccountNumber,
        Register,
        Login,
        SingleChat
    };
    QMap<QString, enum Purpose> map_Switch;  //用于寻找信息是哪个目的
    enum Purpose myPurpose;  //当前任务

    /* 对应的socket */
    QTcpSocket *socket;
    /* 账号检测 */
    QString accountNumber;
    /* User注册 */
    QString password;
};

#endif // MYSUBTHREAD_H
