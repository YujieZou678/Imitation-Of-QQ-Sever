#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QRandomGenerator>

#include "mydatabase.h"

MyDatabase::MyDatabase()
{
    /* 获取一个独一无二的连接名 */
    int randomData;
    QStringList nameList = QSqlDatabase::connectionNames();  //目前所有连接名(有即正在使用)
    bool needGetAgain = false;  //是否需要重新生成随机数
    while (1) {
        QRandomGenerator randomSeed = QRandomGenerator::securelySeeded();  //使用当前时间作为随机种子
        randomData = randomSeed.bounded(0, 100);  //随机数: 0-99
        for (auto name : nameList) {
            if (name == QString::number(randomData)) { needGetAgain = true; break; }  //重新生成随机数
        }
        if (needGetAgain) { needGetAgain = false; continue; }
        break;
    }

    //初始化db
    db = QSqlDatabase::addDatabase("QSQLITE", QString::number(randomData));  //需要创建一个目前没有的连接名
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

QString MyDatabase::checkLogin(const QString &accountNumber, const QString &password)
{
    QSqlQuery query(db);
    query.prepare("select Password from User where AccountNumber = :accountNumber");
    query.bindValue(":accountNumber", accountNumber);
    bool ok = query.exec();
    if (!ok) {
        qDebug() << "Query checkLogin failed:" << query.lastError().text();
    }
    if (query.next()) {
        if (query.value(0) == password) return "true";
    }
    return "false";
}
