/*
function: 服务端的配置，如最大连接数等。
author: zouyujie
date: 2024.3.30
*/
#ifndef MYCONFIG_H
#define MYCONFIG_H

/* 该服务器手动启用3个子线程来管理socket, 并按最优路线分配socket */
/* 大于最大连接时close监听，小于则重新open */

#define SEVER_MAX_CONNECTION 3            //服务器最大可连接数, 为3的倍数, 实际会有>1的误差
#define SEVER_LISTEN_ADDRESS "127.0.0.1"   //服务器监听地址
#define SEVER_LISTEN_PORT 2222             //服务器监听端口

#endif // MYCONFIG_H
