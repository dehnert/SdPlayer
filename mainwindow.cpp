/* thing to do
1)* auto backup
2)* undo
3)* redo
4)* back/forward across file boundaries
5)* open/save playlist files (with relative paths if possible to improve portability)
6)NO open/save group files - probably not, if I save play position in playlist file
7)* loop if loop file exists, loop the loop, otherwise loop the file
8) write a help file
9)* create a program icon
10)* save/restore volume setting in settings ini file
11) compile it on Linux
12)* allow drag/drop playlist files to playlist (or main window)
13)* allow drag/drop folders to playlist (this is just a convenience)
14)* use layout on main window for resizing and stretching
15)* keyboard shortcut to toggle loop mode
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "breaktimer.h"
#include "loop.h"
#include "settings.h"
#include "shortcuts.h"
#include "speed.h"
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QFileDialog>
#include <QCloseEvent>
#include <QProcess>
#include <QTextStream>
#include <vlc/vlc.h>
/*
Things to be aware of when using libvlc to play files:

After creating media with libvlc_media_new_path(),
must call libvlc_media_parse(vlcMedia) to get metadata and duration immediately
(otherwise must wait for libvlc_media_is_parsed(), which will take a while).

When creating a media from a filename, libvlc_media_new_path requires "\" after ":"
such as C:\...
Unfortunately Qt uses all "/" as in Linux, so the "C:/" must be changed to "C:\"

linvlc media player does NOT seek properly when in paused state.
I implemented my own wrappers around the play, pause, get time and set time functions
to keep track if I was in play or pause state, and implement my own "fake seeking" when paused.
I simply keep track of the desired seek position, and delay the actual seek
until the next play(), at which time vlc will seek properly. Problem solved!

For volume control, I make my volume slider be the master that is changed on all volume changes.
Slider value changed that changes the volume in vlc. Never change vlc volume directly!
This method avoids having to poll the vlc volume in my timer to update the slider.
*/
/*
To create a proper Windows Icon file for the Windows EXE file:
First, create an ICO format bitmap file that contains the icon image.
This can be done with e.g. Microsoft Visual C++:
Select File|New, then select the File tab in the dialog that appears, and choose Icon.
(Note that you do not need to load your application into Visual C++; here we are only using the icon editor.)
Or use GIMP to create the ICO file!
Store the ICO file in your application's source code directory,
for example, with the name myappico.ico.
Then, create a text file called, say, myapp.rc in which you put a single line of text:

    IDI_ICON1               ICON    DISCARDABLE     "myappico.ico"

Finally, assuming you are using qmake to generate your makefiles, add this line to your myapp.pro file:

    RC_FILE = myapp.rc

Regenerate your makefile and your application. The .exe file will now be represented with your icon in Explorer.
*/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    playing=false;
    seeked=false;
    Speed=100;
    setWindowIcon(QIcon(QCoreApplication::applicationDirPath()+"/Music.png"));
    // set up the QMap of action names to slots (12 actions for user shortcut keys)
    shortcutSlots["PlayPause"]=SLOT(playpause());
    shortcutSlots["Zero"]=SLOT(zero());
    shortcutSlots["Rewind"]=SLOT(rewind());
    shortcutSlots["Back"]=SLOT(back());
    shortcutSlots["Forward"]=SLOT(forward());
    shortcutSlots["Timer"]=SLOT(tiptimer());
    shortcutSlots["VolumeUp"]=SLOT(volup());
    shortcutSlots["VolumeDown"]=SLOT(voldown());
    shortcutSlots["NextTrack"]=SLOT(next());
    shortcutSlots["PrevTrack"]=SLOT(prev());
    shortcutSlots["Undo"]=SLOT(undo());
    shortcutSlots["Redo"]=SLOT(redo());
    shortcutSlots["Loop"]=SLOT(loop());
    shortcutSlots["BreakTimer"]=SLOT(dobreaktimer());
    emptyUndo();
    queuedFlag=false;
    // create the vlc media framework and two media players (music and tip timer)
    vlcInstance=libvlc_new(0,NULL);
    if (vlcInstance==NULL) {
        QString msg("Create vlc failed: ");
        msg.append(libvlc_errmsg());
        QMessageBox::information(this, "test vlc", msg);
        exit(1);
    }
    vlcPlayer=libvlc_media_player_new(vlcInstance); // create a media player
    vlcPlayerDing=libvlc_media_player_new(vlcInstance); // create a media player for tip timer
    if (vlcPlayer==NULL || vlcPlayerDing==NULL) {
        QMessageBox::information(this,"test vlc","Create media player failed");
        exit(1);
    }
    // connect seek and volume sliders
    connect(ui->horizontalSliderSeek,SIGNAL(sliderMoved(int)),this,SLOT(setPosition(int)));
    connect(ui->horizontalSliderVolume,SIGNAL(valueChanged(int)),this,SLOT(setVolume(int)));
    QTimer *ttimer = new QTimer(this);
    connect(ttimer, SIGNAL(timeout()), this, SLOT(tenthsec()));
    ttimer->start(100); // start executing tenthsec() every 0.1 second
    // set up an empty playlist model
    QStringList list;
    playlistModel = new PlaylistModel(list,this);
    ui->listView_playlist->setModel(playlistModel);
    // make del key a shortcut for deleting entries from playlist
    QAction *del = new QAction(this);
    del->setShortcut(Qt::Key_Delete);
    del->setShortcutContext(Qt::WidgetShortcut); // shortcut enabled only when playlist has focus
    ui->listView_playlist->addAction(del); // add action to the playlist view widget
    connect(del,SIGNAL(triggered()),this,SLOT(deletePlaylistItems()));
    // make double-click a playlist entry play the file
    connect(ui->listView_playlist,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(DoubleclickPlaylist(QModelIndex)));
    // link signals/slots for menus, pushbuttons etc.
    connect(ui->actionOptions,SIGNAL(triggered()),this,SLOT(dosettings()));
    connect(ui->actionShortcuts,SIGNAL(triggered()),this,SLOT(doshortcuts()));
    connect(ui->actionLoop,SIGNAL(triggered()),this,SLOT(doloop()));
    connect(ui->actionBreaktimer,SIGNAL(triggered()),this,SLOT(dobreaktimer()));
    connect(ui->actionSpeed,SIGNAL(triggered()),this,SLOT(dospeed()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(about()));
    connect(ui->actionHelp,SIGNAL(triggered()),this,SLOT(help()));
    connect(ui->pushButton_playpause,SIGNAL(clicked()),this,SLOT(playpause())); // playpause
    connect(ui->pushButton_back,SIGNAL(clicked()),this,SLOT(back())); // back
    connect(ui->pushButton_forward,SIGNAL(clicked()),this,SLOT(forward())); // forward
    connect(ui->pushButton_counter,SIGNAL(clicked()),this,SLOT(zero())); // zero
    connect(ui->pushButton_rewind,SIGNAL(clicked()),this,SLOT(rewind())); // rewind
    connect(ui->pushButton_timer,SIGNAL(clicked()),this,SLOT(tiptimer())); // tip timer
    connect(ui->pushButton_next,SIGNAL(clicked()),this,SLOT(next())); // next
    connect(ui->pushButton_prev,SIGNAL(clicked()),this,SLOT(prev())); // prev
    // volup (no ui button for this, only shortcut key)
    // voldown (no ui button for this, only shortcut key)
    connect(ui->actionUndo,SIGNAL(triggered()),this,SLOT(undo())); // undo
    connect(ui->actionRedo,SIGNAL(triggered()),this,SLOT(redo())); // redo
    readSettings(); // must read the settings
    activateShortcuts(); // before activating shortcuts (shortcuts are in settings!)
    libvlc_audio_set_volume(vlcPlayerDing,TipVolume); // set tip timer volume
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onesec()));
    timer->start(1000); // start executing onesec() every second
    TimerRunning=false; // timer is stopped
    tiptimer(); // reset the tip timer
    connect(playlistModel, SIGNAL(playlistChanged()), this, SLOT(playlistChanged()));
    connect(playlistModel,SIGNAL(playlistDropped(QString)),this,SLOT(playlistDropped(QString)));
    connect(playlistModel,SIGNAL(scrollto(int)),this,SLOT(makerowVisible(int)));
    connect(ui->actionSave_as, SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(save()));
    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closefile()));
    connect(ui->actionOpen_Playlist, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->actionAdd_Tapes, SIGNAL(triggered()), this, SLOT(action_add_tapes()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    PlaylistChanged=false;
    SavedZero.position=-1;
    SavedZero.track=-1;
    playlistModel->setFileTypes(readFileTypes());
// parse the command line for a playlist file or a music file to open
    QStringList ArgList;
    QFileInfo fileinfo;
    ArgList=QCoreApplication::arguments();
    if (ArgList.size()>=2) {
        if (ArgList[1]!="") {
            fileinfo.setFile(ArgList[1]);
            if (fileinfo.suffix().toLower()=="m3u") { // if it is a playlist file
                Openfile=ArgList[1];    // override last open playlist with playlist from command line
            } else {
                if (readFileTypes().contains(fileinfo.suffix().toLower())) { // if file is supported music file type
                    playlistModel->insertRow(0);
                    playlistModel->setData(playlistModel->index(0,0,QModelIndex()),ArgList[1]);
                    Openfile="";    // don't use the previously open playlist (use music file from command line instead)
                    PlaylistChanged=true;
                    emptyUndo(); // empty the zero list
                    markZero(0,0); // and mark the zero location at beginning of the file
                    playpause();
                }
            }
        }
    }
    if (Openfile!="") {
        openPlaylist(Openfile); // this opens the last playlist, or playlist form command line (if any)
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    if (vlcInstance) {
        libvlc_release(vlcInstance); // delete the vlc instance and all vlc media players
    }
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::readSettings()
{
    int size,i;
    struct shortcut newshortcut;

    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"krubow","sdplayer"); // constructor, force INI file storage
    Tiptime=settings.value("tiptime",10).toInt();
    TipVolume=settings.value("tipvolume",100).toInt();
    Backtime=settings.value("backtime",1).toInt();
    Skiptime=settings.value("skiptime",5).toInt();
    Volumestep=settings.value("volumestep",10).toInt();
    Soundfile=settings.value("soundfile").toString();
    Openfile=settings.value("openfile").toString();
    ui->horizontalSliderVolume->setValue(settings.value("volume",100).toInt());
    size=settings.beginReadArray("shortcuts");
    for (i=0; i<size; i++) {
        settings.setArrayIndex(i);
        newshortcut.Key=settings.value("key").toString();
        newshortcut.Action=settings.value("action").toString();
        shortcutlist.append(newshortcut);
    }
    settings.endArray();
}

QStringList MainWindow::readFileTypes(void)
{
    QStringList FileTypes;
    QFile typesfile;

    FileTypes << "mp3";
    typesfile.setFileName(QCoreApplication::applicationDirPath()+"/filetypes.txt");
    if (typesfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream types(&typesfile);
        while (!types.atEnd()) {
            FileTypes << types.readLine().toLower();
        }
    }
    FileTypes.removeDuplicates();
    return FileTypes; // return list of supported file types from filetypes.txt ("mp3" is ALWAYS included)
}

void MainWindow::about(void)
{
    QMessageBox::about(this,"About SdPlayer","<h1>SdPlayer 2.00</h1>using VLC<h2>by Keith Rubow</h2><a href=\"http://www.krubow.com/\">www.krubow.com</a>");
}

void MainWindow::help(void)
{
    QDesktopServices::openUrl(QUrl("file:///"+QCoreApplication::applicationDirPath()+"/sdplayer.html"));
}

QString formatTime(qint64 time)
{
    int minutes,seconds;

    if (time<0) {
        return "-"+formatTime(-time);
    }
    minutes=time/60000;
    seconds=(time-minutes*60000)/1000;
    return QString("%1:%2").arg(minutes).arg(seconds,2,10,QChar('0'));
}

void MainWindow::onesec(void)
{
    if (TimerRunning && Timer) {
        Timer--;
        if (Timer<=0) {
            Timer=0;
            TimerRunning=false;
            if (TimerMode) { // break timer was running
                TimerMode=false;
                playpause(); // playpause will start (or stop) the music
            } else {
                if (Soundfile!="") { // play tip timer sound using vlc
#ifdef _WIN32
                    libvlc_media_t *vlcMedia = libvlc_media_new_path(vlcInstance,Soundfile.replace(":/",":\\").toUtf8().constData()); // create a vlc media
#else
                    libvlc_media_t *vlcMedia = libvlc_media_new_path(vlcInstance,Soundfile.toUtf8().constData()); // create a vlc media
#endif
                    if (vlcMedia) {
                        libvlc_media_player_set_media(vlcPlayerDing,vlcMedia); // set media to play
                        libvlc_media_release(vlcMedia); // release the media
                        libvlc_media_player_play(vlcPlayerDing);
                    }
                }
            }
            ui->pushButton_timer->setStyleSheet("background: red; border: 5px ridge #404000; font-size: 50px;"); // set red
        }
    }
    ui->pushButton_timer->setText(formatTime(Timer*1000)); // formatTime expects milliseconds
}

void MainWindow::deletePlaylistItems()
{
    int flag=0;
    int row,lowrow=-1;

    while (ui->listView_playlist->selectionModel()->selectedIndexes().count()) {
        row=ui->listView_playlist->selectionModel()->selectedIndexes().at(0).row();
        playlistModel->removeRows(row,1);
        if (lowrow<0 || row<lowrow) {
            lowrow=row; // this will be the lowest row number deleted
        }
        flag=1;
    }
    if (flag) {
        playlistChanged();
        if (lowrow>0) {
            makerowVisible(lowrow-1);
//            ui->listView_playlist->scrollTo(playlistModel->index(lowrow-1)); // scroll to make visible file before first playlist item deleted
        }
    }
}
void MainWindow::makerowVisible(int row)
{
    ui->listView_playlist->scrollTo(playlistModel->index(row)); // scroll to make row visible
}

// wrapper to play and execute delayed seek
void MainWindow::play(void)
{
    libvlc_media_player_play(vlcPlayer);
    playing=true;
    if (seeked) { // do delayed seek if necessary
        libvlc_media_player_set_time(vlcPlayer,seektime);
        seeked=false; // no more pending delayed seek
    }
    if (Timer>0) {
        TimerRunning=true; // start timer (if it still has time left)
    }
}

// wrapper to pause and enable fake seeking
void MainWindow::pause(void)
{
    libvlc_media_player_pause(vlcPlayer);
    playing=false; // enable fake seeking
}

// wrapper to get fake or real media time
qint64 MainWindow::gettime(void)
{
    if (seeked) {
        return seektime;
    } else {
        return libvlc_media_player_get_time(vlcPlayer);
    }
}

void MainWindow::settime(qint64 time)
{
    if (playing) {
        libvlc_media_player_set_time(vlcPlayer,time);
    } else {
        seektime=time;
        seeked=true; // there is a pending delayed seek
    }
}

// open nextrack file in media player, position it, and put it in play/pause state per playnow flag
// handle tags and file duration and looping
void MainWindow::playTrack(qint64 position, bool playnow)
{
    QString filename;
    char *string;
    QFileInfo fileinfo;
    QFile loopfile;
    qint64 start,end;

    filename=playlistModel->getFilename(nexttrack); // get the filename
#ifdef _WIN32
    filename.replace(":/",":\\"); // replace forward slash with back slash to make vlc happy on windows
#endif
    libvlc_media_t *vlcMedia = libvlc_media_new_path(vlcInstance,filename.toUtf8().constData()); // create a vlc media
    if (!vlcMedia) {
        return; // error, could not create media
    }
    libvlc_media_parse(vlcMedia); // parse it so I can get metadata and duration
    // get metadata
    string=libvlc_media_get_meta(vlcMedia,libvlc_meta_Artist);
    if (string) {
        ui->label_artist->setText(string);
    } else {
        ui->label_artist->setText("");
    }
    string=libvlc_media_get_meta(vlcMedia,libvlc_meta_Title);
    if (string) {
        ui->label_title->setText(string);
    } else {
        ui->label_title->setText("");
    }
    string=libvlc_media_get_meta(vlcMedia,libvlc_meta_Album);
    if (string) {
        ui->label_album->setText(string);
    } else {
        ui->label_album->setText("");
    }
    duration=libvlc_media_get_duration(vlcMedia); // get duration
    ui->label_length->setText(formatTime(duration));
    ui->horizontalSliderSeek->setMaximum((int)duration); // max duration is 24 days 20:31:23.647
    libvlc_media_player_set_media(vlcPlayer,vlcMedia); // set media to play
    pause(); // to make sure seeks are delayed
    libvlc_media_release(vlcMedia); // release the media
    if (position<0) {
        position+=duration; // position<0 is position from end of file
    }
    settime(position); // seek (delayed)
    if (playnow) {
        play(); // if necessary play and perform delayed seek
    }
    playlistModel->setPlayPos(nexttrack); // set play position
    // now get loop start/end times from the ".loop" file (if it exista)
    fileinfo.setFile(filename);
    filename=fileinfo.path()+"/"+fileinfo.completeBaseName()+".loop";
    loopfile.setFileName(filename);
    if (loopfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream loop(&loopfile);
        start=loop.readLine().toLongLong();
        end=loop.readLine().toLongLong();
        loopfile.close();
        if (start<0) {
            start=0;
        }
        if (end<0) {
            end=0;
        }
        if (end<start) {
            end=start;
        }
    } else {
        start=end=0;
    }
    Loopstart=start;
    Loopend=end;
}

void MainWindow::DoubleclickPlaylist(QModelIndex idx)
{
    nexttrack=idx.row(); // track to play
    playTrack(0,true);
    markZero(nexttrack,0);
}

// tenth second timer is used to update play position indicator
// continue to next track (if any), with auto-zero
// and implement full and partial track looping
void MainWindow::tenthsec(void)
{
    qint64 time,reltime;
    static qint64 oldtime=-1;
    int state;
    static int oldstate=-1;

    state=libvlc_media_player_get_state(vlcPlayer);
    time=gettime();
    if (time<0) {
        time=0;
    }
    if (state==libvlc_Ended) {
        time=0;
    }
    if (time!=oldtime) {
        oldtime=time;
        reltime=time-zeroPosition();
        ui->label_position->setText(formatTime(time));
        ui->pushButton_counter->setText(formatTime(reltime));
        ui->horizontalSliderSeek->setValue(time);
    }
    if (playing && Loopend>0 && ui->checkBox_loop->isChecked()) { // if I partial looping enabled
        if (time>=Loopend) { // if past loop end
            settime(Loopstart); // go back to loop start
        }
    }
    if (state!=oldstate) {
        oldstate=state;
        switch (state) {
        case libvlc_Playing:
            playlistModel->setPlayMode(2); // change playmode so playlist can display proper state
            break;
        case libvlc_Paused:
            playlistModel->setPlayMode(1); // change playmode so playlist can display proper state
            break;
        case libvlc_Ended:
            playlistModel->setPlayMode(0); // change playmode so playlist can display proper state
            if (ui->checkBox_loop->isChecked()) { // if looping complete track
                libvlc_media_player_set_media(vlcPlayer,libvlc_media_player_get_media(vlcPlayer)); // can I just set the same media again?
                play(); // play it again (I don't know why I have to set the same media again)
            } else {
                if (playlistModel->playPos()+1 < playlistModel->rowCount()) { // if there is a next track
                    nexttrack=playlistModel->playPos()+1;
                    playTrack(0,true); // play next track
                    markZero(nexttrack,0);
                } else {
                    playing=false; // no track to play
                    ui->label_artist->setText("");
                    ui->label_title->setText("");
                    ui->label_album->setText("");
                    duration=0;
                    ui->label_length->setText(formatTime(duration));
                    ui->horizontalSliderSeek->setMaximum((int)duration); // max duration is 24 days 20:31:23.647
                    nexttrack=-1;
                    playlistModel->setPlayPos(-1);
                }
            }
            break;
        }
    }
}

void MainWindow::setPosition(int position)
{
    settime(position); // set player position when seek slider moves by mouse
}

void MainWindow::setVolume(int volume)
{
    libvlc_audio_set_volume(vlcPlayer,volume); // set player volume when volume slider changes
}

void MainWindow::loop(void)
{
    ui->checkBox_loop->toggle();
}

void MainWindow::dobreaktimer(void)
{
    Breaktimer dialog(this);
    static int time=50;

    dialog.setTime(time);
    if (dialog.exec()) {
        time=dialog.getTime();
        TimerMode=true; // set break timer mode
        Timer=time*6; // set timer in seconds (convert from tenth minutes)
        TimerRunning=true; // start the timer
        ui->pushButton_timer->setStyleSheet("background: yellow; border: 5px ridge #404000; font-size: 50px;"); // yellow backgroud is break timer indication
    }
}

void MainWindow::doloop(void)
{
    Loop dialog(this);
    QString filename;
    QFileInfo fileinfo;
    QFile loopfile;

    if (playlistModel->playPos()<0) {
        return; // no file playing, cannot set loop
    }
    dialog.setTotalTime(duration);
    if (undoloc>=1) {
        dialog.zeroStart=undolist[undoloc-1].position;
    } else {
        dialog.zeroStart=0;
    }
    if (undoloc>=0) {
        dialog.zeroEnd=undolist[undoloc].position;
    } else {
        dialog.zeroEnd=0;
    }
    dialog.setLoopStart(Loopstart);
    dialog.setLoopEnd(Loopend);
    if (dialog.exec()) {
        Loopstart=dialog.getLoopStart();
        Loopend=dialog.getLoopEnd();
        // write data to the loop file
        filename=playlistModel->getFilename(playlistModel->playPos()); // get the filename
        fileinfo.setFile(filename);
        filename=fileinfo.path()+"/"+fileinfo.completeBaseName()+".loop";
        loopfile.setFileName(filename);
        if (loopfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream loop(&loopfile);
            loop << Loopstart << "\n";
            loop << Loopend << "\n";
            loopfile.close();
        }
    }
}

void MainWindow::dosettings(void)
{
    settings dialog(this);

    dialog.setTiptime(Tiptime);
    dialog.setTipVolume(TipVolume);
    dialog.setBacktime(Backtime);
    dialog.setSkiptime(Skiptime);
    dialog.setVolumestep(Volumestep);
    dialog.setSoundfile(Soundfile);
    dialog.vlcInstance=vlcInstance; // settings needs this to play the tip timer sound
    dialog.vlcPlayerDing=vlcPlayerDing; // settings needs this to play the tip timer sound
    if (dialog.exec()) {
        Tiptime=dialog.getTiptime();
        TipVolume=dialog.getTipVolume();
        Backtime=dialog.getBacktime();
        Skiptime=dialog.getSkiptime();
        Volumestep=dialog.getVolumestep();
        Soundfile=dialog.getSoundfile();
        QSettings settings(QSettings::IniFormat,QSettings::UserScope,"krubow","sdplayer"); // constructor, force INI file storage
        settings.setValue("tiptime",Tiptime);
        settings.setValue("tipvolume",TipVolume);
        settings.setValue("backtime",Backtime);
        settings.setValue("skiptime",Skiptime);
        settings.setValue("volumestep",Volumestep);
        settings.setValue("soundfile",Soundfile);
    }
    libvlc_audio_set_volume(vlcPlayerDing,TipVolume); // set tip timer volume (it was set in dialog, but if cancelled must be restored)
}

void MainWindow::doshortcuts(void)
{
    int size,i;
    shortcuts dialog(this);
    struct shortcut newshortcut;

    size=shortcutlist.size();
    for (i=0; i<size; i++) {
        dialog.append(shortcutlist[i].Key, shortcutlist[i].Action);
    }
    if (dialog.exec()) {
        deactivateShortcuts();
        shortcutlist.clear();
        size=dialog.shortcutsCount();
        for (i=0; i<size; i++) {
            newshortcut.Key=dialog.getKey(i);
            newshortcut.Action=dialog.getAction(i);
            shortcutlist.append(newshortcut);
        }
        activateShortcuts();
        QSettings settings(QSettings::IniFormat,QSettings::UserScope,"krubow","sdplayer"); // constructor, force INI file storage
        settings.beginWriteArray("shortcuts");
        size=shortcutlist.size();
        for (i=0; i<size; i++) {
            settings.setArrayIndex(i);
            settings.setValue("key",shortcutlist[i].Key);
            settings.setValue("action",shortcutlist[i].Action);
        }
        settings.endArray();
    }
}

void MainWindow::dospeed(void)
{
    speed dialog(this);

    dialog.vlcPlayer=vlcPlayer;
    dialog.setSpeed(Speed);
    if (dialog.exec()) {
        Speed=dialog.getSpeed();
    }
    libvlc_media_player_set_rate(vlcPlayer,((float)Speed)/100.0); // change playback speed in case of cancel
}

void MainWindow::activateShortcuts(void)
{
    int i;

    for (i=0; i<shortcutlist.size(); i++) {  // create shortcuts
          connect(shortcutlist[i].ptrShortcut= new QShortcut(shortcutlist[i].Key,this),SIGNAL(activated()),this,shortcutSlots.value(shortcutlist[i].Action));
    }
}

void MainWindow::deactivateShortcuts(void)
{
    int i;

    for (i=0; i<shortcutlist.size(); i++) { // destroy all existing shortcuts
          delete shortcutlist[i].ptrShortcut;
    }
}

// pause if playing (and go back BackTime), else play
void MainWindow::playpause(void)
{
    qint64 position;

    if (playlistModel->playPos()<0) { // no track playing
        next(); // attempt to play next (i.e. first) track
    } else {
        if (playing) { // if playing
            pause();
            position=gettime(); // and back up
            position-=Backtime*1000; // Backtime is seconds, convert to ms
            if (position<0) {
                position=0;
            }
            settime(position);
        } else {
            play();
        }
    }
}

// back Skiptime seconds
// it is complicated by allowing backing up across files, whether playing or paused
void MainWindow::back(void)
{
    qint64 newtime;

    newtime=gettime()-Skiptime*1000; // back up by Skiptime seconds
    if (newtime<0) { // skip back to previous track if possible
        if (playlistModel->playPos()>0) { // if there is a previous track
            nexttrack=playlistModel->playPos()-1;
            playTrack(newtime,playing); // play prev track with negative offset from end, keep play/pause as it was
            return;
        } else { // no previous track
            newtime=0; // skip bback as far as possible in current track
        }
    }
    settime(newtime); // seek
}

// forward Skiptime seconds
void MainWindow::forward(void)
{
    qint64 newtime;

    newtime=gettime()+Skiptime*1000; // back up by Skiptime seconds
    if (newtime>duration) { // skip to next track if possible
        if (playlistModel->playPos()+1 < playlistModel->rowCount()) { // if there is a next track
            nexttrack=playlistModel->playPos()+1;
            playTrack(newtime-duration,playing); // play next track with offset from start, keep play/pause as it was
            return;
        } else { // no next track
            newtime=duration; // skip forward as far as possible in current track
        }
    }
    settime(newtime); // seek
}

/*
The undo list contains a list of positions (track,position)
undoloc is the position of the current zero mark on the list (-1 if none)
The list can have entries with an index>undoloc if undo's have been done.
In this case, a redo is possible, incrementing undoloc up to a max of size-1
*/
void MainWindow::emptyUndo(void)
{
    undoloc=-1;
    undolist.clear();
}

int MainWindow::zeroTrack() // return track of current zero mark, -1 if none
{
    if (undoloc<0) {
        return -1;
    }
    return undolist[undoloc].track;
}

qint64 MainWindow::zeroPosition() // return position of current zero mark, -1 if none
{
    if (undoloc<0) {
        return -1;
    }
    return undolist[undoloc].position;
}

// enter new position in undo list if different from current zero position
void MainWindow::markZero(int track, qint64 position)
{
    struct position here;

    if (track!=zeroTrack() || position!=zeroPosition()) {
        here.track=track;
        here.position=position;
        undoloc++;
        if (undolist.size()>undoloc) { // already room in undolist
            undolist[undoloc]=here;
            while (undolist.size()>undoloc+1) {
                undolist.removeLast();
            }
        } else {
            undolist.append(here);
        }
        ui->pushButton_counter->setText(formatTime(gettime()-zeroPosition())); // update relative position
    }
}

// enter current position in undo list
void MainWindow::zero(void)
{
    markZero(playlistModel->playPos(),gettime()); // zero at current track, current time
}

// seek to current entry in undo list
void MainWindow::rewind(void)
{
    int ztrack;
    qint64 zposition;

    ztrack=zeroTrack();
    zposition=zeroPosition();
    if (playlistModel->playPos()!=ztrack) { // need to change track
        nexttrack=ztrack; // track to play
        playTrack(zposition,playing); // play the track from the zero position (caution, this screws up zero marks if zposition=0)
    } else { // simple case, seek in current track
        settime(zposition);
    }
}

void MainWindow::tiptimer(void)
{
    TimerMode=false; // cancel break timer mode
    if (TimerRunning) { // if running, stop the timer
        TimerRunning=false;
    } else {  // else reset the timer and set background to white
        Timer=Tiptime*60;
        ui->pushButton_timer->setStyleSheet("background: white; border: 5px ridge #404000; font-size: 50px;"); // clear red
        if (playing) {
            TimerRunning=true; // timer must start running if playing
        }
        ui->pushButton_timer->setText(formatTime(Timer*1000)); // formatTime expects milliseconds
    }
}

void MainWindow::next(void)
{
    QString filename;

    nexttrack=playlistModel->playPos()+1;
    if (nexttrack>=playlistModel->rowCount()) {
        nexttrack-=1; // no more tracks
    } else {
        playTrack(0,true);
        markZero(nexttrack,0);
    }
}

void MainWindow::prev(void)
{
    QString filename;

    nexttrack=playlistModel->playPos()-1;
    if (nexttrack<0) {
        nexttrack+=1; // no prev tracks
    } else {
        playTrack(0,true);
        markZero(nexttrack,0);
    }
}

void MainWindow::volup(void)
{
    int volume;

    volume=ui->horizontalSliderVolume->value()+Volumestep;
    if (volume>200) {
        volume=200;
    }
    ui->horizontalSliderVolume->setValue(volume);
}

void MainWindow::voldown(void)
{
    int volume;

    volume=ui->horizontalSliderVolume->value()-Volumestep;
    if (volume<0) {
        volume=0;
    }
    ui->horizontalSliderVolume->setValue(volume);
}

void MainWindow::undo(void)
{
    if (undoloc>0) {
        undoloc--;
        rewind();
    }
}

void MainWindow::redo(void)
{
    if (undoloc+1<undolist.size()) {
        undoloc++;
        rewind();
    }
}

void MainWindow::playlistChanged(void)
{
    PlaylistChanged=true;
}

void MainWindow::setTitle(void)
{
    setWindowTitle(QString("SDplayer ")+QFileInfo(Openfile).fileName()); // display filename without path
}

void MainWindow::saveFile(QString filename)
{
    QFileInfo fileinfo;
    QFile playlistfile;
    QString playlistpath;
    int pathlength;
    QString mp3filename;
    int track;
    qint64 pos;

    fileinfo.setFile(filename);
    playlistpath=fileinfo.path()+"/"; // get path, complete with terminating '/'
    pathlength=playlistpath.size();
    playlistfile.setFileName(filename);
    if (playlistfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream m3u(&playlistfile);
        track=zeroTrack();
        if (track<0) {
            track=0;
        }
        pos=zeroPosition();
        if (pos<0) {
            pos=0;
        }
        m3u << "#TRACK:" << track << "\n";
        m3u << "#POSITION:" << pos << "\n";
        for (int row=0; row<playlistModel->rowCount(); row++) {
            mp3filename=playlistModel->getFilename(row);
            // convert file path to path relative to playlist if possible
            if (mp3filename.startsWith(playlistpath)) {
                mp3filename.remove(0,pathlength);
            }
            m3u << mp3filename << "\n";
        }
        playlistfile.close();
        SavedZero.track=track;
        SavedZero.position=pos;
    }
}

void MainWindow::saveAs(void)
{
    QString filename;
    QFileInfo fileinfo;
    static QString mydir=QDir::homePath(); // last visited directory

    filename=QFileDialog::getSaveFileName(this,"Save Playlist File",mydir,"M3U file (*.m3u)");
    if (filename!="") {
        fileinfo.setFile(filename);
        if (fileinfo.suffix().toLower()!="m3u") {   // if not a "m3u" filename
            filename+=".m3u"; // append .m3u file type
        }
        saveFile(filename);
        Openfile=filename;
        setTitle();
        QDir path(Openfile);
        mydir=path.absolutePath(); // remember last visited directory
        PlaylistChanged=false;
    }
}

void MainWindow::save(void)
{
    if (Openfile=="") {
        saveAs();
    } else {
        saveFile(Openfile);
        PlaylistChanged=false;
    }
}

void MainWindow::closefile(void)
{
    saveAsk();
    if (playlistModel->rowCount()>0) {
        playlistModel->removeRows(0,playlistModel->rowCount());
    }
//    mediaObject->stop(); // don't leave a file playing when I close the playlist
    Openfile="";
    setTitle();
    emptyUndo();
    PlaylistChanged=false;
    SavedZero.track=-1;
    SavedZero.position=-1;
}

void MainWindow::saveAsk(void)
{
    QString message;

    if (PlaylistChanged || (SavedZero.track!=zeroTrack()) || (SavedZero.position!=zeroPosition())) { // playlist changed or  zero point changed
        message=QString("Save changes to playlist?\n%1").arg(Openfile);
        if (QMessageBox::Save==QMessageBox::question(this,"Save File?",message,QMessageBox::Save|QMessageBox::Discard,QMessageBox::Save)) {
            save();
        }
    }
}

void MainWindow::openPlaylist(QString filename)
{
    QFileInfo fileinfo;
    QFile playlistfile;
    QString playlistpath;
    int row;
    QString line;
    int track;
    qint64 position;

    if (filename=="") {
        return; // do nothing if there is no file to open
    }
    closefile(); // must close any open playlist before opening a new one (will prompt to save)
    fileinfo.setFile(filename);
    if (fileinfo.suffix().toLower()!="m3u") {   // if not a "m3u" filename
        filename+=".m3u"; // append .m3u file type
    }
    playlistpath=fileinfo.path()+"/"; // path of playlist file for converting relative mp3 files back to absolute
    playlistfile.setFileName(filename);
    if (playlistfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream m3u(&playlistfile);
        row=0;
        track=0;
        position=0;
        while (!m3u.atEnd()) {
            line=m3u.readLine();
            if (line[0]!='#') {
                fileinfo.setFile(line);
                if (fileinfo.isRelative()) { // if mp3 file has relative path
                    line.prepend(playlistpath); // convert it back to absolute path
                }
                playlistModel->insertRow(row);
                playlistModel->setData(playlistModel->index(row,0,QModelIndex()),line);
                row++;
            } else {
                if (line.startsWith("#TRACK:")) {
                    track=line.remove(0,7).toInt();
                } else {
                    if (line.startsWith("#POSITION:")) {
                        position=line.remove(0,10).toLongLong();
                    }
                }
            }
        }
        playlistfile.close();
        Openfile=filename;
        setTitle();
        emptyUndo();
        markZero(track,position);
        rewind(); // this seeks to the zero location I marked
        PlaylistChanged=false;
        SavedZero.track=track;
        SavedZero.position=position;
    }
}

void MainWindow::open(void)
{
    QString filename;
    static QString mydir=QDir::homePath(); // last visited directory

    filename=QFileDialog::getOpenFileName(this,"Open Playlist File",mydir,"M3U file (*.m3u)");
    if (filename!="") {
        openPlaylist(filename);
        QDir path(filename);
        mydir=path.absolutePath(); // remember last visited directory
    }
}

void MainWindow::action_add_tapes(void)
{
    QStringList filenames;
    static QString mydir=QDir::homePath(); // last visited directory

    filenames=QFileDialog::getOpenFileNames(this,"Add Tapes",mydir,"MP3 files (*.mp3)");
    if (filenames.empty()) return;

    int row = playlistModel->rowCount();
    for (const QString& filename : filenames) {
        QFileInfo fileinfo(filename);
        mydir = fileinfo.dir().absolutePath();
        // TODO: Verify file type (reuse playlistmodel.cpp code?)
        playlistModel->insertRow(row);
        playlistModel->setData(playlistModel->index(row,0,QModelIndex()),filename);
        row++;
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasUrls()) { // if dragged item has list of urls (filenames)
    if (event->mimeData()->urls().size()==1) { // if list contains only a single filename
      event->acceptProposedAction();
    }
  }
}

void MainWindow::dropEvent(QDropEvent *event)
{
  QString filename;
  QFileInfo fileinfo;

  if (event->mimeData()->hasUrls()) {
    foreach (QUrl url, event->mimeData()->urls()) {
      filename=url.toLocalFile();
      fileinfo.setFile(filename); // get information about file
      if (fileinfo.isFile()) { // if it is a file (not a directory or symlink)
        if (fileinfo.suffix().toLower()=="m3u") { // playlist file
            openPlaylist(filename);
        }
      }
    }
  }
}

void MainWindow::playlistDropped(QString filename)
{
    openPlaylist(filename);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveAsk();
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,"krubow","sdplayer"); // constructor, force INI file storage
    settings.setValue("volume",ui->horizontalSliderVolume->value()); // save volume setting
    settings.setValue("openfile",Openfile); // save the open playlist file name
    event->accept();
}
