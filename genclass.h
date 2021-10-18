#ifndef GENCLASS_H
#define GENCLASS_H

#include <QObject>

class GenClass
{
public:
    GenClass();

    static void genHFile(const QString &className, const QList<QString> &keys, const QList<QString> &values);
    static void genCppFile(const QString &className, const QList<QString> &keys, const QList<QString> &values);
};

#endif // GENCLASS_H
