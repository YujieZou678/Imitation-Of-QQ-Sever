/*
function: 重写socket。
author: zouyujie
date: 2024.4.14
*/
#include "mysocket.h"

MySocket::MySocket(QObject *parent) :
    QTcpSocket(parent)
{

}

MySocket::~MySocket()
{
}
