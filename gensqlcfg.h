#ifndef GENSQLCFG_H
#define GENSQLCFG_H

#include <QList>

class TableField;

class GenSqlCfg
{
public:
    GenSqlCfg();

    static void genSqlCfg(const QString &className, const QVector<TableField> &fields, const QStringList &tables);
};

#endif // GENSQLCFG_H
