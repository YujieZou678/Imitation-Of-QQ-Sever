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
    MySubThread(QObject *parent = nullptr);

    void run() override;  //重写，子线程执行

private:

};

#endif // MYSUBTHREAD_H
