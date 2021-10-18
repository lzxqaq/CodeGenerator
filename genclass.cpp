#include "genclass.h"
#include <QApplication>
#include <QStringBuilder>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

#ifdef _WIN32
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif

GenClass::GenClass()
{

}

//void GenClass::genHFile(const QString &className, const QHash<QString, QString> &fields)
void GenClass::genHFile(const QString &className, const QList<QString> &keys, const QList<QString> &values)
{
    QString lowerTmp = className.toLower();
    QString path = QApplication::applicationDirPath() % "/code/" % lowerTmp % "/" % lowerTmp % ".h";
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(Q_NULLPTR, "保存头文件", "文件无法打开，可能被占用");
        return;
    }
    QTextStream stream(&file);


    QString upperTmp = className.toUpper();
    stream<<"#ifndef "<<upperTmp<<"_H"<<NEWLINE;
    stream<<"#define "<<upperTmp<<"_H"<<NEWLINE;
    stream<<NEWLINE;
    stream<<"#include \"genericentity.h\""<<NEWLINE;


    stream<<NEWLINE;
    stream<<"class "<<className<<" : public GenericEntity"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"    Q_OBJECT"<<NEWLINE;

    QStringList varNames;
    QStringList readMethods;
    QStringList writeMethods;
    foreach(QString field, keys) {
        QString tmpName = field.at(0).toLower() % field.right(field.count() - 1);
        varNames<<("m_" % tmpName);
        readMethods<<tmpName;
        writeMethods<<("set" % field);
    }
    //Q_PROPERTY()

    int count = keys.count();
    for(int i=0; i<count; i++) {
        QString propStr = QString("    Q_PROPERTY(%1 %2 READ %3 WRITE %4)").arg(values.at(i))
                .arg(keys.at(i)).arg(readMethods.at(i)).arg(writeMethods.at(i));
        stream<<propStr<<NEWLINE;
    }

    stream<<NEWLINE;
    stream<<"public:"<<NEWLINE;
    //无参构造函数
    stream<<"    "<<className<<"();"<<NEWLINE;

    //数据类接口
//    stream<<NEWLINE;
//    stream<<"    //DataInterface interface"<<NEWLINE;
//    stream<<"    DataInterface* clone() override;"<<NEWLINE;
//    stream<<"    DataInterface *newEmpty() override;"<<NEWLINE;


    //get/set方法
    stream<<NEWLINE;
    stream<<"    //get/set method"<<NEWLINE;
    for(int i=0; i<count; i++) {
        QString getStr = QString("    %1 %2() const;").arg(values.at(i)).arg(readMethods.at(i));
        stream<<getStr<<NEWLINE;
        QString type = values.at(i);
        QString param = type.indexOf(QChar('Q')) == 0 ? QString("const " % type % " &value") : QString(type % " value");
        QString SetStr = QString("    void %1(%2);").arg(writeMethods.at(i)).arg(param);
        stream<<SetStr<<NEWLINE;
        stream<<NEWLINE;
    }

    //私有的成员变量
    stream<<"private:"<<NEWLINE;
    for(int i=0; i<count; i++) {
        QString varStr = QString("    %1 %2;").arg(values.at(i)).arg(varNames.at(i));
        stream<<varStr<<NEWLINE;
    }

    stream<<"};"<<NEWLINE;
    stream<<NEWLINE;
    stream<<NEWLINE;
    stream<<"#endif // "<<upperTmp<<"_H"<<NEWLINE;
    file.close();
}

void GenClass::genCppFile(const QString &className, const QList<QString> &keys, const QList<QString> &values)
{
    QString lowerTmp = className.toLower();
    QString path = QApplication::applicationDirPath() % "/code/" % lowerTmp % "/" % lowerTmp % ".cpp";
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(Q_NULLPTR, "保存源文件", "文件无法打开，可能被占用");
        return;
    }
    QTextStream stream(&file);

    stream<<QString("#include \"%1.h\"").arg(lowerTmp)<<NEWLINE;
//    stream<<"#include <QMetaProperty>"<<NEWLINE;
    stream<<NEWLINE;
    //无参构造函数
    stream<<className<<"::"<<className<<"()"<<NEWLINE;
    stream<<"{"<<NEWLINE;
    stream<<"}"<<NEWLINE;

//    stream<<NEWLINE;
//    stream<<"DataInterface *"<<className<<"::clone()"<<NEWLINE;
//    stream<<"{"<<NEWLINE;
//    stream<<"    "<<className<<" *newData = new "<<className<<"();"<<NEWLINE;
//    stream<<"    const QMetaObject *metaObj = this->metaObject();"<<NEWLINE;
//    stream<<"    int count = metaObj->propertyCount();"<<NEWLINE;
//    stream<<"    for(int i=0; i<count; i++) {"<<NEWLINE;
//    stream<<"        QMetaProperty prop = metaObj->property(i);"<<NEWLINE;
//    stream<<"        const char *name = prop.name();"<<NEWLINE;
//    stream<<"        newData->setProperty(name, this->property(name));"<<NEWLINE;
//    stream<<"    }"<<NEWLINE;
//    stream<<"    return newData;"<<NEWLINE;
//    stream<<"}"<<NEWLINE;

//    stream<<NEWLINE;
//    stream<<"DataInterface *"<<className<<"::newEmpty()"<<NEWLINE;
//    stream<<"{"<<NEWLINE;
//    stream<<"    return new "<<className<<"();"<<NEWLINE;
//    stream<<"}"<<NEWLINE;

    QStringList varNames;
    QStringList readMethods;
    QStringList writeMethods;
    foreach(QString field, keys) {
        QString tmpName = field.at(0).toLower() % field.right(field.count() - 1);
        varNames<<("m_" % tmpName);
        readMethods<<tmpName;
        writeMethods<<("set" % field);
    }

    int count = keys.count();
    for(int i=0; i<count; i++) {
        stream<<NEWLINE;
        stream<<QString("%1 %2::%3() const").arg(values.at(i)).arg(className).arg(readMethods.at(i))<<NEWLINE;
        stream<<"{"<<NEWLINE;
        stream<<"    return "<<varNames.at(i)<<";"<<NEWLINE;
        stream<<"}"<<NEWLINE;
        stream<<NEWLINE;
        QString type = values.at(i);
        QString param = type.indexOf(QChar('Q')) == 0 ? QString("const " % type % " &value") : QString(type % " value");
        stream<<QString("void %1::%2(%3)").arg(className).arg(writeMethods.at(i)).arg(param)<<NEWLINE;
        stream<<"{"<<NEWLINE;
        stream<<"    "<<varNames.at(i)<<" = value;"<<NEWLINE;
        stream<<"}"<<NEWLINE;
    }

    file.close();
}
