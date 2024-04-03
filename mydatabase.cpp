#include <QSqlQuery>
#include <QSqlError>
#include <QThread>

#include "mydatabase.h"

MyDatabase::MyDatabase()
{
    //初始化db
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("myDatabase.db");
    if (!db.open()) {
        qDebug() << "DB open failed:" << db.lastError().text();
    }
}

MyDatabase::~MyDatabase()
{
    db.close();
    QString connectionName = db.connectionName();
    db = QSqlDatabase();  //在这里对db进行一个重置
    QSqlDatabase::removeDatabase(connectionName);  //手动删除连接名
}

void MyDatabase::createUserTable()
{
    QSqlQuery query(db);  //绑定数据库
    bool ok = query.exec("create table User("
                         "Id integer primary key autoincrement,"
                         "AccountNumber text,"
                         "Password text)");
    if (!ok) {
        qDebug() << "Query createTable failed:" << query.lastError().text();
    }
}

QString MyDatabase::checkAccountNumber(const QString &accountNumber)
{
    QSqlQuery query(db);
    bool ok = query.exec("select AccountNumber from User");
    if (!ok) {
        qDebug() << "Query checkAccountNumber failed:" << query.lastError().text();
    }
    while (query.next()) {
        if (query.value(0) == accountNumber) return "true";
    }
    return "false";
}

void MyDatabase::addUser(const QString &accountNumber, const QString &password)
{
    QSqlQuery query(db);
    query.prepare("insert into User(AccountNumber, Password) values(:accountNumber, :password)");
    query.bindValue(":accountNumber", accountNumber);
    query.bindValue(":password", password);
    bool ok = query.exec();
    if (!ok) {
        qDebug() << "Query addUser failed:" << query.lastError().text();
        return;
    }
}
