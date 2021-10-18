#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QRadioButton;

struct TableField
{
    TableField()
    {
        isNullable = false;
        isPk = false;
        length = 0;
        isForeignKey = false;
    }

    QString name;
    QString typeName;
    bool isNullable;
    bool isPk;
    int length;
    QString table;

    bool isForeignKey;//是否外键
    QString foreignName;//在外表的名字
    QString foreignTable;//外表

    QString alias;//别名，用在显示和生成的名称
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void radioButtonTrueOn(QRadioButton *rdo);
    QString typeMap(QString typeName);//数据库类型映射为Qt数据类型
    void createDir(QString className);//每个类生成后都会放在对应的文件夹中
    bool isValid();//检验结果表是否合法，能够正确生成代码和使用

private slots:
    void on_btnTestConn_clicked();

    void on_btnConnect_clicked();

    void on_btnSetMainTable_clicked();

    void on_btnSetForeignTable_clicked();

    void on_btnReset_clicked();

    void on_btnGenResult_clicked();

    void on_btnDelField_clicked();

    void on_btnOpenDir_clicked();

    void on_btnGenClass_clicked();

    void on_btnGenSqlCfg_clicked();

    void on_btnGenJsonCfg_clicked();

    void on_btnGenSelectAllSql_clicked();

    void on_btnFindDown_clicked();

    void on_btnFindUp_clicked();

    void on_btnDelFT_clicked();

    void on_btnSaveFieldName_clicked();

    void on_btnGenDao_clicked();

private:
    Ui::MainWindow *ui;

    QListWidget *m_tableListWidget;//数据库表（名）的列表
    QListWidget *m_mainTableWidget;//主表的字段（名）的列表
    QListWidget *m_foreignTableWidget;//外表和外键的关系的列表
    QListWidget *m_resultTableWidget;//左连接生成的字段的列表

    QSqlDatabase m_db;
    QString m_driverType;
    QString m_connSql;
    QString m_setTableStructSql;
    QVector<TableField> m_fields;
    QStringList m_tables;//主表和所有外表
};
#endif // MAINWINDOW_H
