#include "gendao.h"
#include <QApplication>
#include <QStringBuilder>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "mainwindow.h"

#ifdef _WIN32
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif

GenDao::GenDao()
{

}

void GenDao::genHFile(const QString &className, const QVector<TableField> &fields, const QStringList &tables)
{
    QString lowerTmp = className.toLower();
    QString path = QApplication::applicationDirPath() % "/code/" % lowerTmp % "/" % lowerTmp % "dao.h";
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(Q_NULLPTR, "保存头文件", "文件无法打开，可能被占用");
        return;
    }

    QTextStream stream(&file);

    QString upperTmp = className.toUpper();
    stream<<"#ifndef "<<upperTmp<<"DAO_H"<<NEWLINE;
    stream<<"#define "<<upperTmp<<"DAO_H"<<NEWLINE;
    stream<<NEWLINE;
    stream<<"#include <QObject>"<<NEWLINE;

    stream<<NEWLINE;

    stream<<"class ";
    stream<<className;

    stream<<NEWLINE;
    stream<<"class "<<className<<"Dao"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"public:"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"    "<<className<<" selectById(const QVariant &id) const;"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"    "<<"QList<"<<className<<"> selectList() const;"<<NEWLINE;


    stream<<NEWLINE;
    stream<<"    "<<"bool insert(const "<<className<<" &entity) const;"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"    "<<"bool batchInsert(const QVector<"<<className<<"> &entities) const;"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"    "<<"bool update(const "<<className<<" &entity) const;"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"    "<<"bool batchUpdate(const QVector<"<<className<<"> &entities) const;"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"    "<<"bool deleteById(const QVariant &id) const;"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"    "<<"bool batchDelete(const QVector<QVariant> &ids) const;"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"    "<<"int count() const;"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"};"<<NEWLINE;
    stream<<NEWLINE;
    stream<<"#endif // "<<upperTmp<<"DAO_H"<<NEWLINE;
    file.close();
}

void GenDao::genCppFile(const QString &className, const QVector<TableField> &fields, const QStringList &tables)
{
    QString lowerTmp = className.toLower();
    QString path = QApplication::applicationDirPath() % "/code/" % lowerTmp % "/" % lowerTmp % "dao.cpp";
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(Q_NULLPTR, "保存源文件", "文件无法打开，可能被占用");
        return;
    }

    QString key;
    QStringList fieldNames;
    foreach (TableField tf, fields)
    {
        fieldNames << tf.name;
        if(tf.isPk) {
            key = tf.name;
        }
    }

    QTextStream stream(&file);

    stream<<QString("#include \"%1dao.h\"").arg(lowerTmp)<<NEWLINE;
    stream<<QString("#include \"%1.h\"").arg(lowerTmp)<<NEWLINE;
    stream<<QString("#include \"DbUtil\"")<<NEWLINE;

    stream<<NEWLINE;
    stream<<"const QString SQL_NAMESPACE = \"";
    stream<<className;
    stream<<"\";";
    stream<<NEWLINE;

    stream<<NEWLINE;
    stream<<className<<" "<<className<<"Dao::selectById(const QVariant &id) const"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"    QString sql = SqlHandler::instance().getSql(SQL_NAMESPACE, \"selectById\");"<<NEWLINE;
    stream<<"    QVariantMap paramMap;"<<NEWLINE;
    stream<<"    paramMap[\""<<key<<"\"] = id;"<<NEWLINE;
    stream<<"    return DBUtil().selectBean<"<<className<<">(sql, paramMap);"<<NEWLINE;
    stream<<"}"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"QList<"<<className<<"> "<<className<<"Dao::selectList() const"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"    QString sql = SqlHandler::instance().getSql(SQL_NAMESPACE, \"selectList\");"<<NEWLINE;
    stream<<"    return DBUtil().selectBeans<"<<className<<">(sql);"<<NEWLINE;
    stream<<"}"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"bool "<<className<<"Dao::insert(const "<<className<<" &entity) const"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"    QString sql = SqlHandler::instance().getSql(SQL_NAMESPACE, \"insert\");"<<NEWLINE;
    stream<<"    QVariantMap paramMap;"<<NEWLINE;
    foreach (auto fieldName, fieldNames)
    {
        stream<<"    paramMap[\""<<fieldName<<"\"] = entity.property(\""<<fieldName<<"\");"<<NEWLINE;
    }
    stream<<"    return DBUtil().update(sql, paramMap);"<<NEWLINE;
    stream<<"}"<<NEWLINE;


    stream<<NEWLINE;
    stream<<"bool "<<className<<"Dao::batchInsert(const QVector<"<<className<<"> &entities) const"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"    QString sql = SqlHandler::instance().getSql(SQL_NAMESPACE, \"insert\");"<<NEWLINE;
    stream<<"    QList<QVariantMap> paramMaps;"<<NEWLINE;
    stream<<NEWLINE;
    stream<<"    foreach ("<<className<<" entity, entities)"<<NEWLINE;
    stream<<"    {"<<NEWLINE;
    stream<<"        QVariantMap paramMap;"<<NEWLINE;
    foreach (auto fieldName, fieldNames)
    {
        stream<<"        paramMap[\""<<fieldName<<"\"] = entity.property(\""<<fieldName<<"\"); ";
    }
    stream<<"        paramMaps.append(paramMap);"<<NEWLINE;
    stream<<"    }"<<NEWLINE;
    stream<<NEWLINE;
    stream<<"    return DBUtil().updateBatch(sql, paramMaps);"<<NEWLINE;
    stream<<"}"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"bool "<<className<<"Dao::update(const "<<className<<" &entity) const"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"    QString sql = SqlHandler::instance().getSql(SQL_NAMESPACE, \"update\");"<<NEWLINE;
    stream<<"    QVariantMap paramMap;"<<NEWLINE;
    foreach (auto fieldName, fieldNames)
    {
        stream<<"    paramMap[\""<<fieldName<<"\"] = entity.property(\""<<fieldName<<"\");"<<NEWLINE;
    }
    stream<<"    return DBUtil().update(sql, paramMap);"<<NEWLINE;
    stream<<"}"<<NEWLINE;


    stream<<NEWLINE;
    stream<<"bool "<<className<<"Dao::batchUpdate(const QVector<"<<className<<"> &entities) const"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"    QString sql = SqlHandler::instance().getSql(SQL_NAMESPACE, \"insert\");"<<NEWLINE;
    stream<<"    QList<QVariantMap> paramMaps;"<<NEWLINE;
    stream<<NEWLINE;
    stream<<"    foreach ("<<className<<" entity, entities)"<<NEWLINE;
    stream<<"    {"<<NEWLINE;
    stream<<"        QVariantMap paramMap;"<<NEWLINE;
    foreach (auto fieldName, fieldNames)
    {
        stream<<"        paramMap[\""<<fieldName<<"\"] = entity.property(\""<<fieldName<<"\");"<<NEWLINE;
    }
    stream<<"        paramMaps.append(paramMap);"<<NEWLINE;
    stream<<"    }"<<NEWLINE;
    stream<<NEWLINE;
    stream<<"    return DBUtil().updateBatch(sql, paramMaps);"<<NEWLINE;
    stream<<"}"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"bool "<<className<<"Dao::deleteById(const QVariant &id) const"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"    QString sql = SqlHandler::instance().getSql(SQL_NAMESPACE, \"deleteById\");"<<NEWLINE;
    stream<<"    QVariantMap paramMap;"<<NEWLINE;
    stream<<"    paramMap[\""<<key<<"\"] = id; "<<NEWLINE;
    stream<<"    return DBUtil().update(sql, paramMap);"<<NEWLINE;
    stream<<"}"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"bool "<<className<<"Dao::batchDelete(const QVector<QVariant> &ids) const"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"    QString sql = SqlHandler::instance().getSql(SQL_NAMESPACE, \"deleteById\");"<<NEWLINE;
    stream<<"    QList<QVariantMap> paramMaps;"<<NEWLINE;
    stream<<NEWLINE;
    stream<<"    foreach (QVariant id, ids)"<<NEWLINE;
    stream<<"    {"<<NEWLINE;
    stream<<"        QVariantMap paramMap;"<<NEWLINE;
    stream<<"        paramMap[\""<<key<<"\"] = id; "<<NEWLINE;
    stream<<"        paramMaps.append(paramMap);"<<NEWLINE;
    stream<<"    }"<<NEWLINE;
    stream<<NEWLINE;
    stream<<"    return DBUtil().updateBatch(sql, paramMaps);"<<NEWLINE;
    stream<<"}"<<NEWLINE;

    stream<<NEWLINE;
    stream<<"int "<<className<<"Dao::count() const"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"    QString sql = SqlHandler::instance().getSql(SQL_NAMESPACE, \"count\");"<<NEWLINE;
    stream<<"    return DBUtil().selectInt(sql);"<<NEWLINE;
    stream<<"}"<<NEWLINE;
    stream<<NEWLINE;

    file.close();
}
