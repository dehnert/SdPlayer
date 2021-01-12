#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QDialog>

namespace Ui {
    class shortcuts;
}

class shortcuts : public QDialog {
    Q_OBJECT
public:
    shortcuts(QWidget *parent = 0);
    ~shortcuts();
    void append(QString key, QString action);
    int shortcutsCount(void);
    QString getKey(int row);
    QString getAction(int row);

protected:
    void changeEvent(QEvent *e);

public slots:
  void additem(void);
  void deleteitem(void);

private:
    Ui::shortcuts *ui;
};

#endif // SHORTCUTS_H
