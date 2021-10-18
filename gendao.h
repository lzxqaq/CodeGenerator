#ifndef GENDAO_H
#define GENDAO_H

#include <QObject>

class TableField;

class GenDao
{
public:
    GenDao();

    static void genHFile(const QString &className, const QVector<TableField> &fields, const QStringList &tables);
    static void genCppFile(const QString &className, const QVector<TableField> &fields, const QStringList &tables);
};

#endif // GENDAO_H
