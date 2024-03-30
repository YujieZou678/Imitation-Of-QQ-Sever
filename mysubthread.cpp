/*
function: 线程池调用的。用于子线程启用线程池处理请求。
author: zouyujie
date: 2024.3.25
*/
#include <QThread>

#include "mysubthread.h"

MySubThread::MySubThread(QObject *parent) :
    QObject(parent)
{
    //
}

void MySubThread::run()
{
    //
}
