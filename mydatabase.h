/*
function: 数据库的处理。
author: zouyujie
date: 2024.3.25
*/
#ifndef MYDATABASE_H
#define MYDATABASE_H

#include <QSqlDatabase>

class MyDatabase
{
public:
    MyDatabase();
    ~MyDatabase();

    void createUserTable();  //创建表
    QString checkAccountNumber(const QString&);           //检测账号
    QString addUser(const QString&, const QString &);     //添加用户(注册)
    QString checkLogin(const QString&, const QString &);  //检测登陆信息

private:
    QSqlDatabase db;  //数据库连接对象
};

#endif // MYDATABASE_H
