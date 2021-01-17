#include "shortcut.h"
#include "ui_shortcut.h"

shortcut::shortcut(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::shortcut)
{
    ui->setupUi(this);
}

shortcut::~shortcut()
{
    delete ui;
}

void shortcut::changeEvent(QEvent *e)
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

QString shortcut::getKey(void)
{
    QString key;

    key=ui->comboBox_key->currentText();
    if (ui->checkBox_ctrl->isChecked()) {
      key.prepend("Ctrl+");
    }
    if (ui->checkBox_shift->isChecked()) {
      key.prepend("Shift+");
    }
    if (ui->checkBox_alt->isChecked()) {
      key.prepend("Alt+");
    }

    return key;
}

QString shortcut::getAction(void)
{
    return ui->comboBox_action->currentText();
}
