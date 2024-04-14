/*
function: 子线程。用于管理并监听主线程分下来socket。
author: zouyujie
date: 2024.3.27
*/
#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include "mysocket.h"

class QThreadPool;
class QSettings;

class MyThread : public QObject
{
    Q_OBJECT

public:
    MyThread(QObject *parent = nullptr);
    ~MyThread();

    void addOneSocket(qintptr socketDescriptor);  //添加一个socket
    QString getIp_Port(MySocket*);  //获得一个socket的ip_port
    void savePersonlInfo(const QJsonDocument&, const QByteArray&data);  //缓存个人信息
    QByteArray getProfileImage();  //获取头像

    int socketCount{0};   //当前正在管理的socket数量
    int ID;  //属于第几个线程

signals:
    /* 子线程与主线程通信 */
    void needQuitThread();  //当管理socket数量为0时需要关闭线程
    void needCloseListen(); //关闭server监听
    void needOpenListen();  //打开server监听

    /* 子线程间的通信 */
    void toThread2_SendMsg(const QString&);  //给线程2发信息

public slots:
    void onPrintThreadStart();  //打印子线程已启动
    void onReceiveFromSubThread(const QString&);  //接收来自子线程的信息
    void onFinished_CheckAccountNumber(MySocket *, const QString&);  //接收来自线程池账号检测的信息
    void onFinished_Login(MySocket *, const QString&);  //接收来自线程池登陆检测的信息
    void onFinished_PrepareSendFile(MySocket *socket);  //返回发送文件的信息
    void onFinished_SendFile(MySocket *socket);  //返回接收文件完成

private:
    enum class Purpose {  //枚举(class内部使用)
        CheckAccountNumber,
        Register,
        Login,
        PrepareSendFile,
        SingleChat
    };

    QMap<QString,MySocket*> socketsMap;    //容器管理socket
    QMap<QString, enum Purpose> map_Switch;  //用于寻找信息是哪个目的
    QThreadPool *myThreadPool;  //获取当前程序的线程池对象
    QSettings *settings;  //缓存数据对象
};

#endif // MYTHREAD_H
