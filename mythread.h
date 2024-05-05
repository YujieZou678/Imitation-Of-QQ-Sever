/*
function: 子线程。用于管理并监听主线程分下来socket。
author: zouyujie
date: 2024.3.27
*/
#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include <QJsonDocument>
#include "mysocket.h"

class QThreadPool;
class QSettings;

class MyThread : public QObject
{
    Q_OBJECT

public:
    MyThread(QObject *parent = nullptr);
    ~MyThread();

    void addOneSocket(qintptr socketDescriptor);              //添加一个socket
    QString getIp_Port(MySocket*);                            //获得一个socket的ip_port
    void saveProfileImage(const QString&, const QByteArray&data, qintptr);  //缓存个人头像
    QByteArray getProfileImageData(const QString&);                         //获取头像数据
    int getProfileImageSize(const QString&);                                //获取头像大小
    void startSendFile(MySocket *, const QByteArray &_data);  //开始传输文件
    void savePersonalData(const QJsonDocument&);              //缓存个人信息
    QString getPersonalData(const QString&, const QString&);  //获取个人信息
    void saveFriendData(const QJsonDocument&);                //缓存好友信息(加好友)
    QJsonArray getFriendChatHistory(const QString&, const QString&);//获取聊天记录
    QJsonArray getFriendArray(const QString&);                //获取好友列表

    int socketCount{0};   //当前正在管理的socket数量
    int ID;               //属于第几个线程

signals:
    /* 子线程与主线程通信 */
    void needQuitThread();  //当管理socket数量为0时需要关闭线程
    void needCloseListen(); //关闭server监听
    void needOpenListen();  //打开server监听

    /* 子线程间的通信 */
    void toSubThread1_SendMsg(const QJsonDocument&);  //给子线程1发信息
    void toSubThread2_SendMsg(const QJsonDocument&);  //给子线程2发信息
    void toSubThread3_SendMsg(const QJsonDocument&);  //给子线程3发信息

public slots:
    void onPrintThreadStart();                           //打印"子线程已启动"
    void onFinished_CheckAccountNumber(MySocket *, const QJsonDocument&);  //账号检测完毕
    void onFinished_CheckGroupNumber(MySocket *, const QJsonDocument&);    //群账号检测完毕
    void onFinished_Register(MySocket *, const QString&);//注册完毕
    void onFinished_Login(MySocket *, const QString &, const QString &);   //登陆检测完毕
    void onFinished_PrepareSendFile(MySocket *socket);   //返回发送文件的信息
    void onFinished_SendFile(MySocket *socket);          //返回接收文件完成

    void onReceiveFromSubThread(const QJsonDocument&);   //接收来自子线程的信息

private:
    enum class Purpose {  //枚举(class内部使用)
        CheckAccountNumber,
        Register,
        Login,
        PrepareSendFile,
        ReceiveFile,
        ChangePersonalData,
        AddFriend,
        RequestGetProfileAndName,
        SaveChatHistory,
        GetChatHistory,
        CreateGroup,

        /* 子线程间的通信 */
        RefreshFriendList,
        TransmitMsg
    };

    QMap<QString,MySocket*> socketsMap;      //容器管理socket
    QMap<QString,MySocket*> accountSocketsMap;//账号对应socket, 同上
    QMap<QString, enum Purpose> map_Switch;  //用于寻找信息是哪个目的
    QThreadPool *myThreadPool;  //获取当前程序的线程池对象
    QSettings *settings;        //缓存数据对象

    /* 用于传输大文件 */
//    QVector<QThread*> threads;
//    QVector<MyThread*> myThreads;  //子线程们
};

#endif // MYTHREAD_H
