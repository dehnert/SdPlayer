#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QDialog>

namespace Ui {
    class shortcut;
}

class shortcut : public QDialog {
    Q_OBJECT
public:
    shortcut(QWidget *parent = 0);
    ~shortcut();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::shortcut *ui;

public:
    QString getKey(void);
    QString getAction(void);
};

#endif // SHORTCUT_H
