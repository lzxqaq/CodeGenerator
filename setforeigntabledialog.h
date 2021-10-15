#ifndef SETFOREIGNTABLEDIALOG_H
#define SETFOREIGNTABLEDIALOG_H

#include <QDialog>

namespace Ui {
class SetForeignTableDialog;
}

struct TableField;

class SetForeignTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetForeignTableDialog(QWidget *parent = nullptr);
    ~SetForeignTableDialog();

    void initData(const QVector<TableField> &fields, const QString &foreignTable, const QVector<TableField> &foreignFields);

    QVector<TableField> allFields() const;

    QVector<TableField> foreignFields() const;

private slots:
    void on_btnOk_clicked();

    void on_btnCancel_clicked();

    void on_btnSetForeignKey_clicked();

    void on_btnSetFTKey_clicked();

    void on_btnFindDown_clicked();

private:
    Ui::SetForeignTableDialog *ui;

    QVector<TableField> m_allFields;//不包含此外表的字段
    QVector<TableField> m_foreignFields;

    int m_leftIndex;//左边外键在m_allFields的索引
    int m_rightIndex;//右边主键在m_foreignFields的索引
};

#endif // SETFOREIGNTABLEDIALOG_H
