/*
function: 服务端的配置，如最大连接数等。
author: zouyujie
date: 2024.3.30
*/
#ifndef MYCONFIG_H
#define MYCONFIG_H

/* 该服务器手动启用5个子线程来管理socket, 并按最优路线分配socket */

#define SEVER_MAX_CONNECTION 10            //服务器最大可连接数
#define SEVER_LISTEN_ADDRESS "127.0.0.1"   //服务器监听地址
#define SEVER_LISTEN_PORT 2222             //服务器监听端口

#endif // MYCONFIG_H
