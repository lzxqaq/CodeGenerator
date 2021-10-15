#include "gensqlcfg.h"
#include <QApplication>
#include <QtXml>
#include <QMessageBox>
#include <QStringBuilder>
#include "mainwindow.h"

GenSqlCfg::GenSqlCfg()
{

}

void GenSqlCfg::genSqlCfg(const QString &className, const QVector<TableField> &fields, const QStringList &tables)
{
    QString lowerTmp = className.toLower();
    QString path = QApplication::applicationDirPath() % "/code/" % lowerTmp % "/" % lowerTmp % ".xml";
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(Q_NULLPTR, "保存qsl配置文件", "文件无法打开，可能被占用");
        return;
    }
    QString fieldsStr;
    QString fieldsStr2;
    QString keyFieldsStr;
    foreach(TableField tf, fields) {
        fieldsStr = fieldsStr % tf.name % ", ";
        fieldsStr2 = fieldsStr2 % ":" % tf.name % ", ";
        if(tf.isPk) {
            keyFieldsStr = keyFieldsStr % tf.name % "=:" % tf.name % " AND ";
        }
    }
    fieldsStr = fieldsStr.left(fieldsStr.length() - 2);
    fieldsStr2 = fieldsStr2.left(fieldsStr2.length() - 2);
    keyFieldsStr = keyFieldsStr.left(keyFieldsStr.length() - 5);
    
    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.setCodec("UTF-8");
    stream.writeStartDocument();
    stream.writeStartElement("sqls");
    stream.writeAttribute("namespace", className);
    stream.writeStartElement("define");
    stream.writeAttribute("id", "fields");
    stream.writeCharacters(fieldsStr);
    stream.writeEndElement();

    if(tables.count() == 1) {
        stream.writeStartElement("sql");
        stream.writeAttribute("id", "query");
        stream.writeCharacters("SELECT ");
        stream.writeStartElement("include");
        stream.writeAttribute("defineId", "fields");
        stream.writeEndElement();
        QString quertStr = QString(" FROM %1 WHERE %2").arg(tables.at(0)).arg(keyFieldsStr);
        stream.writeCharacters(quertStr);
        stream.writeEndElement();

        stream.writeStartElement("sql");
        stream.writeAttribute("id", "insert");
        QString insertStr = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(tables.at(0)).arg(fieldsStr).arg(fieldsStr2);
        stream.writeCharacters(insertStr);
        stream.writeEndElement();
    }

    stream.writeEndElement();
    stream.writeEndDocument();
    file.close();
}
