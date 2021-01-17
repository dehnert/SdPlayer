#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vlc/vlc.h>
#include <QShortcut>
#include "playlistmodel.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void about(void);
    void help(void);
    void onesec(void);
    void tenthsec(void);
    void setPosition(int position);
    void setVolume(int volume);
    void deletePlaylistItems();
    void makerowVisible(int row);
    void DoubleclickPlaylist(QModelIndex idx);
    void loop(void);
    void dobreaktimer(void);
    void doloop(void);
    void dosettings(void);
    void doshortcuts(void);
    void dospeed(void);
    void playpause(void);
    void back(void);
    void forward(void);
    void zero(void);
    void rewind(void);
    void tiptimer(void);
    void next(void);
    void prev(void);
    void volup(void);
    void voldown(void);
    void undo(void);
    void redo(void);
    void playlistChanged(void);
    void saveAs(void);
    void save(void);
    void closefile(void);
    void open(void);
    void dragEnterEvent(QDragEnterEvent *event); // for drag-drop
    void playlistDropped(QString filename);

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event); // this lets me "trap" any window close event

private:
    Ui::MainWindow *ui;
    PlaylistModel *playlistModel;
    libvlc_instance_t *vlcInstance; // instance of the vlc media framework, created on program startup
    libvlc_media_player_t *vlcPlayer,*vlcPlayerDing; // two media players, vlcPlayer for music, vlcPlayerDing for tip timer sound
    int nexttrack;
    qint64 duration;
    qint64 Loopstart,Loopend;
    QString Soundfile; // sound file for timer
    int Tiptime; // timer interval (minutes)
    int TipVolume; // volume of tip timer
    int Backtime; // auto backup time (seconds)
    int Skiptime; // skip forward/back time (seconds)
    int Volumestep; // volume step (parts per hundred)
    int Speed; // 75-125 in percent (not saved on exit)
    struct position {
        int track;
        qint64 position;
    };
    QList<struct position> undolist; // undo history buffer
    int undoloc; // current position in undolist
    // current zero position is undolist[undoloc];
    bool playing; // flag that file is in play mode
    bool seeked; // flag that seektime has been set by seek while not playing
    qint64 seektime; // my fake seek time for seeking while not playing
    qint64 queuedPosition; // position to seek to in new file (after sourceChanged), negative is offset from end of file
    bool queuedFlag; // flag that a queuedPosition seek is pending
    bool loopedFlag; // flag that file looped and prefinish mark need to be set again
    // data for the customizable shortcut keys
    QString Openfile;
    struct shortcut {
          QString Key;
          QString Action;
          QShortcut *ptrShortcut;
    };
    QList<struct shortcut> shortcutlist;
    QMap<QString, const char *> shortcutSlots;
    bool TimerRunning;
    bool TimerMode; // true for break timer mode
    int Timer;
    bool PlaylistChanged; // flag that playlist has been altered since opening/saving it
    struct position SavedZero;  // zero position saved in current playlist file

    void makeVisible(int row);
    void readSettings(void);
    void play(void);
    void pause(void);
    qint64 gettime(void);
    void settime(qint64 time);
    void playTrack(qint64 position, bool play);
    void emptyUndo(void);
    int zeroTrack();
    qint64 zeroPosition();
    void markZero(int track, qint64 position);
    void activateShortcuts(void);
    void deactivateShortcuts(void);
    void saveFile(QString filename);
    void setTitle(void);
    void saveAsk(void);
    void openPlaylist(QString filename);
    void dropEvent(QDropEvent *event); // for drag-drop
    QStringList readFileTypes(void);
};

#endif // MAINWINDOW_H
