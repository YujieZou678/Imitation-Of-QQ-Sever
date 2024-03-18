/*
function: 仿QQ服务端。
author: zouyujie
date: 2024.3.18
*/
#include "tcpserver.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TcpServer w;
    return a.exec();
}
