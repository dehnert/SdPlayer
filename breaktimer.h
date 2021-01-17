#ifndef BREAKTIMER_H
#define BREAKTIMER_H

#include <QDialog>

namespace Ui {
    class Breaktimer;
}

class Breaktimer : public QDialog
{
    Q_OBJECT

public:
    explicit Breaktimer(QWidget *parent = 0);
    ~Breaktimer();
    void setTime(int time);
    int getTime(void);

public slots:
    void sliderchanged(int val);
    void spinboxchanged(double val);

private:
    Ui::Breaktimer *ui;
};

#endif // BREAKTIMER_H
