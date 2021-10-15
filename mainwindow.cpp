#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QtSql>
#include <QMessageBox>
#include <QVBoxLayout>
#include "setforeigntabledialog.h"
#include "genclass.h"
#include "gensqlcfg.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->rdoMySql, &QRadioButton::toggled, this, [=](bool checked) {
        if(checked) {
            radioButtonTrueOn(ui->rdoMySql);
        }
    });
    connect(ui->rdoPostgreSql, &QRadioButton::toggled, this, [=](bool checked) {
        if(checked) {
            radioButtonTrueOn(ui->rdoPostgreSql);
        }
    });
    connect(ui->rdoSqlServer, &QRadioButton::toggled, this, [=](bool checked) {
        if(checked) {
            radioButtonTrueOn(ui->rdoSqlServer);
        }
    });
    connect(ui->rdoSqlite, &QRadioButton::toggled, this, [=](bool checked) {
        if(checked) {
            radioButtonTrueOn(ui->rdoSqlite);
        }
    });

    ui->rdoSqlServer->setChecked(true);

    ui->letFindTable->setAttribute(Qt::WA_InputMethodEnabled, false);//关闭中文输入法

    QVBoxLayout *vLayout = new QVBoxLayout();
    m_tableListWidget = new QListWidget(this);
    vLayout->addWidget(m_tableListWidget);
    ui->tableListFrame->setLayout(vLayout);

    vLayout = new QVBoxLayout();
    m_mainTableWidget = new QListWidget(this);
    m_mainTableWidget->setWordWrap(true);
    vLayout->addWidget(m_mainTableWidget);
    ui->mainTableFrame->setLayout(vLayout);

    vLayout = new QVBoxLayout();
    m_foreignTableWidget = new QListWidget(this);
    m_foreignTableWidget->setWordWrap(true);
    vLayout->addWidget(m_foreignTableWidget);
    ui->foreignTableFrame->setLayout(vLayout);

    vLayout = new QVBoxLayout();
    m_resultTableWidget = new QListWidget(this);
    m_resultTableWidget->setWordWrap(true);
    connect(m_resultTableWidget, &QListWidget::currentRowChanged, this, [=](int currentRow) {
        if(currentRow == -1) {
            ui->letFieldName->setText("");
        }
        else {
            ui->letFieldName->setText(m_fields.at(currentRow).name);
        }
    });
    vLayout->addWidget(m_resultTableWidget);
    ui->resultTableFrame->setLayout(vLayout);

    QRegExp regExp("^[a-zA-Z]\\w+");//只能输入字母、数字、下划线，且开头是字母
    ui->letClassName->setValidator(new QRegExpValidator(regExp, this));
    ui->letClassName->setAttribute(Qt::WA_InputMethodEnabled, false);//关闭中文输入法

    ui->letFieldName->setValidator(new QRegExpValidator(regExp, this));
    ui->letFieldName->setAttribute(Qt::WA_InputMethodEnabled, false);//关闭中文输入法
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::radioButtonTrueOn(QRadioButton *rdo)
{
    if(rdo == ui->rdoMySql) {
        ui->letConnect->setText("");
        m_driverType = "QMYSQL";
        m_connSql = "select name from sysobjects where xtype='U' order by name";
    }
    else if(rdo == ui->rdoPostgreSql) {
        ui->letConnect->setText("");
        m_driverType = "QPSQL";
        m_connSql = "select name from sysobjects where xtype='U' order by name";
    }
    else if(rdo == ui->rdoSqlite) {
        ui->letConnect->setText("");
        m_driverType = "QSQLITE";
        m_connSql = "select name from sysobjects where xtype='U' order by name";
    }
    else if(rdo == ui->rdoSqlServer) {
        ui->letConnect->setText("DRIVER={SQL SERVER};SERVER=127.0.0.1,1433;DATABASE=CARMES10_5Y;UID=CARMES6User;PWD=CA.RME!S6U#*ser");
        m_driverType = "QODBC";
        m_connSql = "select name from sysobjects where xtype='U' order by name";
        m_setTableStructSql = "SELECT CASE WHEN col.colorder = 1 THEN obj.name " \
                                  "ELSE '' " \
                                  "END AS tablename, " \
                                  "col.colorder AS orderid, " \
                                  "col.name AS name, " \
                                  "t.name AS typename, " \
                                  "col.length AS length, " \
                                  "CASE WHEN EXISTS (SELECT 1 " \
                                  "FROM dbo.sysindexes si " \
                                      "INNER JOIN dbo.sysindexkeys sik ON si.id = sik.id " \
                                          "AND si.indid = sik.indid " \
                                      "INNER JOIN dbo.syscolumns sc ON sc.id = sik.id " \
                                          "AND sc.colid = sik.colid " \
                                      "INNER JOIN dbo.sysobjects so ON so.name = si.name " \
                                          "AND so.xtype = 'PK' " \
                                  "WHERE sc.id = col.id " \
                                      "AND sc.colid = col.colid ) THEN '1' " \
                                  "ELSE '' " \
                                  "END AS iskey, " \
                                  "CASE WHEN col.isnullable = 1 THEN '1' " \
                                  "ELSE '' " \
                                  "END AS isnullable " \
                              "FROM dbo.syscolumns col " \
                                  "LEFT JOIN dbo.systypes t ON col.xtype = t.xusertype " \
                                  "inner JOIN dbo.sysobjects obj ON col.id = obj.id " \
                                      "AND obj.xtype = 'U' " \
                                      "AND obj.status >= 0 " \
                                  "LEFT JOIN dbo.syscomments comm ON col.cdefault = comm.id " \
                                  "LEFT JOIN sys.extended_properties ep ON col.id = ep.major_id " \
                                      "AND col.colid = ep.minor_id " \
                                      "AND ep.name = 'MS_Description' " \
                                 "LEFT JOIN sys.extended_properties epTwo ON obj.id = epTwo.major_id " \
                                      "AND epTwo.minor_id = 0 " \
                                      "AND epTwo.name = 'MS_Description' " \
                              "WHERE obj.name = '%1' " \
                                  "ORDER BY col.colorder ";
    }
}

void MainWindow::on_btnTestConn_clicked()
{
    QString connectionName = ui->letConnect->text().trimmed();
    QSqlDatabase db = QSqlDatabase::addDatabase(m_driverType);
    db.setDatabaseName(connectionName);
    if(db.open()) {
        statusBar()->showMessage("测试连接数据库成功", 3000);
    }
    else {
        statusBar()->showMessage("测试连接数据库失败：" + db.lastError().text());
    }
}

void MainWindow::on_btnConnect_clicked()
{
    QString connectionName = ui->letConnect->text().trimmed();
    m_db = QSqlDatabase::addDatabase(m_driverType);
    m_db.setDatabaseName(connectionName);
    if(m_db.open()) {
        statusBar()->showMessage("连接数据库成功");

        QSqlQuery query(m_db);
        QString sql = m_connSql;
        if(query.exec(sql)) {
            m_tableListWidget->clear();
            m_mainTableWidget->clear();
            m_foreignTableWidget->clear();
            m_resultTableWidget->clear();

            QStringList tableList;
            while (query.next()) {
                m_tableListWidget->addItem(query.value(0).toString());
                tableList<<query.value(0).toString().toLower();
            }
            m_tableListWidget->setProperty("name", tableList);
            m_tableListWidget->setCurrentRow(0);
        }
        else {
            statusBar()->showMessage("数据库查询失败：" + query.lastError().text());
        }
    }
    else {
        statusBar()->showMessage("连接数据库失败：" + m_db.lastError().text());
        QSqlQuery query(m_db);
        QString sql = QString("");
    }
}

void MainWindow::on_btnSetMainTable_clicked()
{
    if(m_db.isOpen()) {
        m_mainTableWidget->clear();
        m_foreignTableWidget->clear();
        m_fields.clear();
        m_tables.clear();

        QListWidgetItem *item = m_tableListWidget->currentItem();
        QString name = item->text();
        m_tables<<name;

        QSqlQuery query(m_db);
        QString sql = m_setTableStructSql.arg(name);
        if(query.exec(sql)) {
            m_mainTableWidget->addItem(name);
            if(ui->rdoMySql->isChecked()) {

            }
            else if(ui->rdoPostgreSql->isChecked()) {

            }
            else if(ui->rdoSqlite->isChecked()) {

            }
            else if(ui->rdoSqlServer->isChecked()) {
                while (query.next()) {
                    TableField tf;
                    tf.name = query.value(2).toString();
                    tf.typeName = query.value(3).toString();
                    tf.isNullable = query.value(6).toBool();
                    tf.isPk = query.value(5).toBool();
                    tf.length = query.value(4).toInt();
                    tf.table = name;
                    tf.alias = tf.name;
                    m_fields.append(tf);

                    QString fieldStr = QString("%1, %2, %3, %4").arg(tf.name).arg(tf.typeName).arg(tf.isNullable).arg(tf.length);
                    m_mainTableWidget->addItem(fieldStr);
                }
            }
        }
        else {
            statusBar()->showMessage("数据库查询失败：" + query.lastError().text());
        }
    }
    else {
        statusBar()->showMessage("数据库连接断开，请重新连接");
    }
}

void MainWindow::on_btnSetForeignTable_clicked()
{
    if(m_db.isOpen()) {
        if(m_mainTableWidget->count() == 0) {
            statusBar()->showMessage("请先设置主表");
            return;
        }

        QListWidgetItem *item = m_tableListWidget->currentItem();
        QString name = item->text();
        if(m_tables.at(0) == name) {
            statusBar()->showMessage("主表不作为自身的外表");
            return;
        }

        QSqlQuery query(m_db);
        QString sql = m_setTableStructSql.arg(name);
        QVector<TableField> foreignFields;
        if(query.exec(sql)) {
            if(ui->rdoMySql->isChecked()) {

            }
            else if(ui->rdoPostgreSql->isChecked()) {

            }
            else if(ui->rdoSqlite->isChecked()) {

            }
            else if(ui->rdoSqlServer->isChecked()) {
                while (query.next()) {
                    TableField tf;
                    tf.name = query.value(2).toString();
                    tf.typeName = query.value(3).toString();
                    tf.isNullable = query.value(6).toBool();
                    tf.length = query.value(4).toInt();
                    tf.table = name;
                    tf.alias = tf.name;
                    foreignFields.append(tf);
                }
            }

            if(!m_tables.contains(name)) {
                m_tables<<name;
            }
            if(foreignFields.count() > 0) {
                SetForeignTableDialog *dialog = new SetForeignTableDialog(this);
                dialog->setWindowModality(Qt::ApplicationModal);
                dialog->initData(m_fields, name, foreignFields);
                dialog->show();
                connect(dialog, &QDialog::accepted, this, [=] {
                    QVector<TableField> afs = dialog->allFields();
                    QVector<TableField> ffs = dialog->foreignFields();

                    m_fields.swap(afs);
                    m_fields.append(ffs);

                    int count = m_tables.count();
                    int fCount = m_fields.count();
                    m_foreignTableWidget->clear();
                    for(int i=1; i<count; i++) {
                        int index = 0;
                        m_foreignTableWidget->addItem("--------------------");
                        for(; index < fCount; index++) {
                            const TableField tf = m_fields.at(index);
                            if(tf.isForeignKey && tf.foreignTable == m_tables.at(i)) {
                                QString tableStr = QString("%1 << %2(%3)").arg(m_tables.at(i)).arg(tf.table).arg(tf.name);
                                m_foreignTableWidget->addItem(tableStr);
                                QString fieldStr = QString("%1, %2, %3, %4").arg(tf.foreignName).arg(tf.typeName).arg(tf.isNullable).arg(tf.length);
                                m_foreignTableWidget->addItem(fieldStr);
                                break;
                            }
                        }
                        for(; index < fCount; index++) {
                            const TableField tf = m_fields.at(index);
                            if(tf.table == m_tables.at(i)) {
                                QString fieldStr = QString("%1, %2, %3, %4").arg(tf.name).arg(tf.typeName).arg(tf.isNullable).arg(tf.length);
                                m_foreignTableWidget->addItem(fieldStr);
                            }
                        }
                    }
                });
            }
        }
        else {
            statusBar()->showMessage("数据库查询失败：" + query.lastError().text());
        }

    }
    else {
        statusBar()->showMessage("数据库连接断开，请重新连接");
    }
}

void MainWindow::on_btnDelFT_clicked()
{
    int row = m_foreignTableWidget->currentRow();
    QString text = m_foreignTableWidget->currentItem()->text();
    QString name = text.split(" ").at(0);

    if(!m_tables.contains(name)) {
        statusBar()->showMessage("没有选中的外表");
        return;
    }
    int button = QMessageBox::question(this, "删除外表", "删除外表可能会出现问题，是否继续删除？",
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if(button == QMessageBox::No) {
        return;
    }

    int count = m_fields.count();
    QList<int> removeList;
    for(int i=count-1; i>=0; i--) {
        const TableField &tf = m_fields.at(i);
        if(tf.isForeignKey && tf.foreignTable == name) {
            m_fields[i].isForeignKey = false;//移除外键关联
        }
        else if(tf.table == name) {
            removeList.append(i);
        }
    }
    foreach(int i, removeList) {
        m_fields.removeAt(i);
    }

    int rowCount = removeList.count() + 3;//外表在m_foreignTableWidget占用的行数
    for(int i=0; i<rowCount; i++) {
         m_foreignTableWidget->takeItem(row);
    }
    if(row < m_foreignTableWidget->count()) {
        m_foreignTableWidget->scrollToItem(m_foreignTableWidget->item(row));
    }
    else {
        m_foreignTableWidget->scrollToItem(m_foreignTableWidget->item(row - 1));
    }
    m_tables.removeOne(name);
}

void MainWindow::on_btnReset_clicked()
{
    m_mainTableWidget->clear();
    m_foreignTableWidget->clear();
    m_fields.clear();
    m_tables.clear();
}

void MainWindow::on_btnGenResult_clicked()
{
    if(m_tables.count() == 0) {
        return;
    }

    m_resultTableWidget->clear();
    QRegExp regExp("^[a-zA-Z](\\w| |,)+");//且开头是字母

    if(ui->rdoMySql->isChecked()) {

    }
    else if(ui->rdoPostgreSql->isChecked()) {

    }
    else if(ui->rdoSqlite->isChecked()) {

    }
    else if(ui->rdoSqlServer->isChecked()) {
        foreach(TableField tf, m_fields) {
            QString resultStr = QString("%1, %2, %3").arg(tf.alias).arg(typeMap(tf.typeName)).arg(tf.table);
            QListWidgetItem *item = new QListWidgetItem(resultStr);
            m_resultTableWidget->addItem(item);
        }
    }
}

void MainWindow::on_btnDelField_clicked()
{
    int row = m_resultTableWidget->currentRow();

    if(row < 0) {
        statusBar()->showMessage("没有选中的字段");
        return;
    }
    if(m_fields.at(row).isForeignKey) {
        int button = QMessageBox::question(this, "删除字段", "删除外键字段可能会出现问题，是否继续删除？",
                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(button == QMessageBox::No) {
            return;
        }
    }

    m_resultTableWidget->takeItem(row);
    if(row < m_resultTableWidget->count()) {
        m_resultTableWidget->scrollToItem(m_resultTableWidget->item(row));
        m_resultTableWidget->setCurrentRow(row);
    }
    else {
        m_resultTableWidget->scrollToItem(m_resultTableWidget->item(row - 1));
        m_resultTableWidget->setCurrentRow(row - 1);
    }
    m_fields.removeAt(row);
}

void MainWindow::on_btnOpenDir_clicked()
{
    QString path = QApplication::applicationDirPath() % "/code/";
    if(!QDir(path).exists()) {
        QDir().mkpath(path);
    }
    QDesktopServices::openUrl(QUrl(path));
}

void MainWindow::on_btnGenClass_clicked()
{
    if(isValid()) {
        createDir(ui->letClassName->text());

//        QHash<QString, QString> hash;
        QList<QString> keys;
        QList<QString> values;
        int count = m_fields.count();
        for(int i=0; i<count; i++) {
//            hash.insert(m_fields.at(i).name, typeMap(m_fields.at(i).typeName));
            keys << m_fields.at(i).name;
            values << typeMap(m_fields.at(i).typeName);
        }

        GenClass::genHFile(ui->letClassName->text(), keys, values);
        GenClass::genCppFile(ui->letClassName->text(), keys, values);

        statusBar()->showMessage("数据类生成成功");
    }
}

void MainWindow::on_btnGenSqlCfg_clicked()
{
    if(isValid()) {
        createDir(ui->letClassName->text());

        GenSqlCfg::genSqlCfg(ui->letClassName->text(), m_fields, m_tables);

        statusBar()->showMessage("数据类sql配置文件生成成功");
    }
}

void MainWindow::on_btnGenJsonCfg_clicked()
{

}

void MainWindow::on_btnGenSelectAllSql_clicked()
{

}

void MainWindow::on_btnFindDown_clicked()
{
    int row = m_tableListWidget->currentRow();

    const QStringList &list = qvariant_cast<QStringList>(m_tableListWidget->property("name"));
    int count = list.count();
    QString text = ui->letFindTable->text().toLower();
    for(int i=row+1; i<count; i++) {
        if(list.at(i).contains(text)) {
            QListWidgetItem *item = m_tableListWidget->item(i);
            m_tableListWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
            m_tableListWidget->setCurrentRow(i);
            return;
        }
    }

    for(int i=0; i<row; i++) {
        if(list.at(i).contains(text)) {
            QListWidgetItem *item = m_tableListWidget->item(i);
            m_tableListWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
            m_tableListWidget->setCurrentRow(i);
            return;
        }
    }

    statusBar()->showMessage("无匹配的表名", 3000);
}

void MainWindow::on_btnFindUp_clicked()
{
    int row = m_tableListWidget->currentRow();

    const QStringList &list = qvariant_cast<QStringList>(m_tableListWidget->property("name"));
    int count = list.count();
    QString text = ui->letFindTable->text().toLower();
    for(int i=row-1; i>=0; i--) {
        if(list.at(i).contains(text)) {
            QListWidgetItem *item = m_tableListWidget->item(i);
            m_tableListWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
            m_tableListWidget->setCurrentRow(i);
            return;
        }
    }

    for(int i=count-1; i>=row; i--) {
        if(list.at(i).contains(text)) {
            QListWidgetItem *item = m_tableListWidget->item(i);
            m_tableListWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
            m_tableListWidget->setCurrentRow(i);
            return;
        }
    }

    statusBar()->showMessage("无匹配的表名", 3000);
}

QString MainWindow::typeMap(QString typeName)
{
    if(ui->rdoMySql->isChecked()) {

    }
    else if(ui->rdoPostgreSql->isChecked()) {

    }
    else if(ui->rdoSqlite->isChecked()) {

    }
    else if(ui->rdoSqlServer->isChecked()) {
        if(typeName == QStringLiteral("float") || typeName == QStringLiteral("double")) {
            return QStringLiteral("qreal");
        }
        else if(typeName == QStringLiteral("int")) {
            return QStringLiteral("int");
        }
        else if(typeName == QStringLiteral("datetime")) {
            return QStringLiteral("QDateTime");
        }
        else if(typeName == QStringLiteral("image") || typeName == QStringLiteral("binary")) {
            return QStringLiteral("QByteArray");
        }
        else if(typeName.contains(QStringLiteral("char")) || typeName == QStringLiteral("text")) {
            return QStringLiteral("QString");
        }
    }

    return QStringLiteral("QString");
}

void MainWindow::createDir(QString className)
{
    QString path = QApplication::applicationDirPath() % "/code/" % className.toLower();
    if(!QDir(path).exists()) {
        QDir().mkpath(path);
    }
}

bool MainWindow::isValid()
{
    //类名不能为空
    if(ui->letClassName->text().count() == 0) {
        statusBar()->showMessage("类名不能为空");
        return false;
    }

    //数据表不合法
    if(m_fields.count() == 0 || m_tables.count() == 0 || m_resultTableWidget->count() == 0) {
        statusBar()->showMessage("数据表不合法");
        return false;
    }

    QStringList tableList;
    tableList<<m_tables.at(0);
    foreach(TableField tf, m_fields) {
        if(tf.isForeignKey && !tableList.contains(tf.foreignTable)) {
            tableList<<tf.foreignTable;
        }
    }
    //关联外表的外键缺失
    //存在同名字段
    int count = m_fields.count();
    for(int i=0; i<count; i++) {
        if(tableList.count() != m_tables.count() && !tableList.contains(m_fields.at(i).table)) {
            statusBar()->showMessage("关联外表" % m_fields.at(i).table % "的外键缺失");
            return false;
        }
        for(int j=0; j<count; j++) {
            if(i != j && m_fields.at(i).alias == m_fields.at(j).alias) {
                statusBar()->showMessage("存在同名字段" % m_fields.at(i).alias);
                return false;
            }
        }
    }

    return true;
}

void MainWindow::on_btnSaveFieldName_clicked()
{
    //字段重命名
    int row = m_resultTableWidget->currentRow();
    if(row != -1 && ui->letFieldName->text().count() > 0) {
        QString text = ui->letFieldName->text();
        m_fields[row].alias = text;
        QString resultStr = QString("%1, %2, %3").arg(m_fields.at(row).alias).arg(typeMap(m_fields.at(row).typeName)).arg(m_fields.at(row).table);
        m_resultTableWidget->currentItem()->setText(resultStr);
    }
}
