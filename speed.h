#ifndef SPEED_H
#define SPEED_H

#include <QDialog>
#include <vlc/vlc.h>

namespace Ui {
    class speed;
}

class speed : public QDialog
{
    Q_OBJECT

public:
    explicit speed(QWidget *parent = 0);
    ~speed();
    void setSpeed(int Speed);
    int getSpeed(void);
    libvlc_media_player_t *vlcPlayer; // media player vlcPlayer for changing speed

public slots:
    void changeSpeed(int Speed);
    void displaySpeed(int Speed);

private:
    Ui::speed *ui;
};

#endif // SPEED_H
