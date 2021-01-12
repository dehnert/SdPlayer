#include "shortcuts.h"
#include "ui_shortcuts.h"
#include "shortcut.h"

shortcuts::shortcuts(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::shortcuts)
{
    ui->setupUi(this);
    connect(ui->pushButton_delete,SIGNAL(clicked()),this,SLOT(deleteitem()));
    connect(ui->pushButton_add,SIGNAL(clicked()),this,SLOT(additem()));
}

shortcuts::~shortcuts()
{
    delete ui;
}

void shortcuts::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void shortcuts::additem(void)
{
    shortcut dialog(this);

    if (dialog.exec()) {
        append(dialog.getKey(), dialog.getAction());
    }
}

void shortcuts::deleteitem(void)
{
    ui->tableWidget_shortcuts->removeRow(ui->tableWidget_shortcuts->currentRow());
}

// append key,action to tableWidget_shortcuts
void shortcuts::append(QString key, QString action)
{
    int rows;

    rows=ui->tableWidget_shortcuts->rowCount();
    ui->tableWidget_shortcuts->insertRow(rows);
    QTableWidgetItem *item=new QTableWidgetItem(key);
    ui->tableWidget_shortcuts->setItem(rows,0,item);
    item=new QTableWidgetItem(action);
    ui->tableWidget_shortcuts->setItem(rows,1,item);
}

int shortcuts::shortcutsCount(void)
{
    return ui->tableWidget_shortcuts->rowCount();
}

QString shortcuts::getKey(int row)
{
    return ui->tableWidget_shortcuts->item(row,0)->text();
}

QString shortcuts::getAction(int row)
{
    return ui->tableWidget_shortcuts->item(row,1)->text();
}
