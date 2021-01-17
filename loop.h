#ifndef LOOP_H
#define LOOP_H

#include <QDialog>

namespace Ui {
    class Loop;
}

class Loop : public QDialog {
    Q_OBJECT
public:
    Loop(QWidget *parent = 0);
    ~Loop();
    void setTotalTime(int totalTime);
    void setLoopStart(int loopStart);
    int getLoopStart(void);
    void setLoopEnd(int loopEnd);
    int getLoopEnd(void);
    int zeroStart,zeroEnd;

public slots:
    void setStart(void);
    void setEnd(void);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Loop *ui;
};

#endif // LOOP_H
