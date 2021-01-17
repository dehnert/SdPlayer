#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <vlc/vlc.h>

namespace Ui {
    class settings;
}

class settings : public QDialog {
    Q_OBJECT
public:
    settings(QWidget *parent = 0);
    ~settings();
    int getTiptime(void);
    void setTiptime(int Tiptime);
    int getTipVolume(void);
    void setTipVolume(int volume);
    int getBacktime(void);
    void setBacktime(int Backtime);
    int getSkiptime(void);
    void setSkiptime(int Skiptime);
    int getVolumestep(void);
    void setVolumestep(int Volumestep);
    QString getSoundfile(void);
    void setSoundfile(QString Soundfile);
    libvlc_instance_t *vlcInstance; // instance of the vlc media framework
    libvlc_media_player_t *vlcPlayerDing; // media player vlcPlayerDing for tip timer sound

public slots:
    void browse(void);
    void playsound(void);
    void changeVolume(int volume);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::settings *ui;
};

#endif // SETTINGS_H
