/*
function: 仿QQ服务端。
author: zouyujie
date: 2024.3.18
*/
#include <QApplication>
#include <QMainWindow>

#include "mytcpserver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMainWindow w;  //窗口是为了完整退出程序
    w.show();

    MyTcpServer myTcpServer;
    return a.exec();
}
