#include "setforeigntabledialog.h"
#include "ui_setforeigntabledialog.h"
#include "mainwindow.h"

SetForeignTableDialog::SetForeignTableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetForeignTableDialog)
{
    ui->setupUi(this);

    this->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    this->

    ui->twAll->setColumnCount(2);
    ui->twAll->setColumnWidth(0, 300);
    ui->twForeign->setColumnCount(2);
    ui->twForeign->setColumnWidth(0, 200);

    ui->letFindTable->setAttribute(Qt::WA_InputMethodEnabled, false);//关闭中文输入法
}

SetForeignTableDialog::~SetForeignTableDialog()
{
    delete ui;
}

void SetForeignTableDialog::initData(const QVector<TableField> &fields, const QString &foreignTable, const QVector<TableField> &foreignFields)
{
    m_allFields.append(fields);
    m_foreignFields.append(foreignFields);
    QStringList removeNames;

    //移除外表的字段
    QList<int> removeList;
    int count = m_allFields.count();
    for(int i=count-1; i>=0; i--) {
        if(m_allFields.at(i).foreignTable == foreignTable) {
            m_allFields[i].isForeignKey = false;
        }
        else if(m_allFields.at(i).table == foreignTable) {
            removeList.append(i);
        }
    }
    foreach(int i, removeList) {
        removeNames.append(m_allFields.takeAt(i).name);
    }

    //显示已有字段
    QStringList headerLabels;
    headerLabels<<"字段"<<"可设外键";
    ui->twAll->setHorizontalHeaderLabels(headerLabels);
    QTableWidgetItem *item = Q_NULLPTR;
    QStringList fieldList;
    count = m_allFields.count();
    for(int i=0; i<count; i++) {
        ui->twAll->insertRow(i);
        const TableField &tf = m_allFields.at(i);
        QString s = QString("%1, %2 (%3)").arg(tf.name).arg(tf.typeName).arg(tf.table);
        item = new QTableWidgetItem(s);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui->twAll->setItem(i, 0, item);
        fieldList<<tf.name.toLower();

        item = new QTableWidgetItem();
        //float、image类型不会是外表主键的类型，已经是外键的不会关联第二个表
        bool unChecked = tf.typeName == QStringLiteral("float") || tf.typeName == QStringLiteral("image")
                || tf.isForeignKey;
        item->setCheckState(unChecked ? Qt::Unchecked : Qt::Checked);
        item->setFlags(Qt::NoItemFlags);
        ui->twAll->setItem(i, 1, item);
    }
    ui->twAll->setProperty("name", fieldList);

    //外表字段
    headerLabels.clear();
    headerLabels<<"字段"<<"是否使用";
    ui->twForeign->setHorizontalHeaderLabels(headerLabels);
    ui->twForeign->insertRow(0);//插入首行，显示外表名
    item = new QTableWidgetItem(foreignTable);
    ui->twForeign->setItem(0, 0, item);

    count = m_foreignFields.count();
    for(int i=1; i<=count; i++) {
        const TableField &tf = m_foreignFields.at(i - 1);
        ui->twForeign->insertRow(i);
        QString s = QString("%1, %2, %3, %4").arg(tf.name).arg(tf.typeName).arg(tf.isNullable).arg(tf.length);
        item = new QTableWidgetItem(s);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui->twForeign->setItem(i, 0, item);

        item = new QTableWidgetItem();
        item->setCheckState(removeNames.contains(tf.name) ? Qt::Checked : Qt::Unchecked);
        ui->twForeign->setItem(i, 1, item);
    }
    if(count > 0) {
        ui->letFTKey->setText(m_foreignFields.at(0).name + ", " + m_foreignFields.at(0).typeName);
        m_rightIndex = 0;
    }
}

void SetForeignTableDialog::on_btnOk_clicked()
{
    if(m_leftIndex < 0 || m_rightIndex < 0) {
        return;
    }

    TableField &lf = m_allFields[m_leftIndex];
    TableField &rf = m_foreignFields[m_rightIndex];
    if(lf.typeName == rf.typeName) {
        lf.isForeignKey = true;
        lf.foreignName = rf.name;
        lf.foreignTable = rf.table;

        QList<int> removeList;
        int count = m_foreignFields.count();
        for(int i=count-1; i>=0; i--) {
            if(ui->twForeign->item(i + 1, 1)->checkState() == Qt::Unchecked) {
                removeList.append(i);
            }
        }
        //外表主键已存储为外键
        if(ui->twForeign->item(m_rightIndex + 1, 1)->checkState() == Qt::Checked) {
            removeList.append(m_rightIndex);
        }
        foreach(int i, removeList) {
            m_foreignFields.removeAt(i);
        }
    }
    else {
        return;
    }

    accept();
}

void SetForeignTableDialog::on_btnCancel_clicked()
{
    reject();
}

void SetForeignTableDialog::on_btnSetForeignKey_clicked()
{
    int row = ui->twAll->currentRow();
    if(row >= 0 && ui->twAll->item(row, 1)->checkState() == Qt::Checked) {
        ui->letForeignKey->setText(m_allFields.at(row).name+ ", " + m_allFields.at(row).typeName + ", " + m_allFields.at(row).table);
        m_leftIndex = row;
    }
}

void SetForeignTableDialog::on_btnSetFTKey_clicked()
{
    int row = ui->twForeign->currentRow();
    if(row > 0) {
        ui->letFTKey->setText(m_foreignFields.at(row - 1).name + ", " + m_foreignFields.at(row - 1).typeName);
        m_rightIndex = row - 1;
    }
}

QVector<TableField> SetForeignTableDialog::allFields() const
{
    return m_allFields;
}

QVector<TableField> SetForeignTableDialog::foreignFields() const
{
    return m_foreignFields;
}

void SetForeignTableDialog::on_btnFindDown_clicked()
{
    int row = ui->twAll->currentRow();

    const QStringList &list = qvariant_cast<QStringList>(ui->twAll->property("name"));
    int count = list.count();
    QString text = ui->letFindTable->text().toLower();
    for(int i=row+1; i<count; i++) {
        if(list.at(i).contains(text)) {
            QTableWidgetItem *item = ui->twAll->item(i, 0);
            ui->twAll->scrollToItem(item, QAbstractItemView::EnsureVisible);
            ui->twAll->setCurrentCell(i, 0);
            return;
        }
    }

    for(int i=0; i<row; i++) {
        if(list.at(i).contains(text)) {
            QTableWidgetItem *item = ui->twAll->item(i, 0);
            ui->twAll->scrollToItem(item, QAbstractItemView::EnsureVisible);
            ui->twAll->setCurrentCell(i, 0);
            return;
        }
    }
}
