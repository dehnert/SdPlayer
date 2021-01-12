#include "settings.h"
#include "ui_settings.h"
#include <QFileDialog>

settings::settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings)
{
    ui->setupUi(this);
    connect(ui->pushButton_browse,SIGNAL(clicked()),this,SLOT(browse()));
    connect(ui->pushButtonTest,SIGNAL(clicked()),this,SLOT(playsound()));
    connect(ui->horizontalSliderVolume,SIGNAL(sliderMoved(int)),this,SLOT(changeVolume(int)));
}

settings::~settings()
{
    delete ui;
}

void settings::changeEvent(QEvent *e)
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

void settings::browse(void)
{
      QString filename;

      filename=QFileDialog::getOpenFileName(this,"Open Sound File","","");
      ui->lineEdit_soundfile->setText(filename);
}

void settings::changeVolume(int volume)
{
    libvlc_audio_set_volume(vlcPlayerDing,volume); // set tip timer volume
}

void settings::playsound(void)
{
#ifdef _win32
    libvlc_media_t *vlcMedia = libvlc_media_new_path(vlcInstance,ui->lineEdit_soundfile->text().replace(":/",":\\").toUtf8().constData()); // create a vlc media
#else
    libvlc_media_t *vlcMedia = libvlc_media_new_path(vlcInstance,ui->lineEdit_soundfile->text().toUtf8().constData()); // create a vlc media
#endif
    if (vlcMedia) {
        libvlc_media_player_set_media(vlcPlayerDing,vlcMedia); // set media to play
        libvlc_media_release(vlcMedia); // release the media
        libvlc_media_player_play(vlcPlayerDing); // play the sound
    }
}

int settings::getTiptime(void)
{
    return ui->spinBox_tipTimer->value();
}

void settings::setTiptime(int Tiptime)
{
    ui->spinBox_tipTimer->setValue(Tiptime);
}

int settings::getTipVolume(void)
{
    return ui->horizontalSliderVolume->value();
}

void settings::setTipVolume(int volume)
{
    ui->horizontalSliderVolume->setValue(volume);
}

int settings::getBacktime(void)
{
    return ui->spinBox_backTime->value();
}

void settings::setBacktime(int Backtime)
{
    ui->spinBox_backTime->setValue(Backtime);
}

int settings::getSkiptime(void)
{
    return ui->spinBox_skipTime->value();
}

void settings::setSkiptime(int Skiptime)
{
    ui->spinBox_skipTime->setValue(Skiptime);
}

int settings::getVolumestep(void)
{
    return ui->spinBox_volumeStep->value();
}

void settings::setVolumestep(int Volumestep)
{
    ui->spinBox_volumeStep->setValue(Volumestep);
}

QString settings::getSoundfile(void)
{
    return ui->lineEdit_soundfile->text();
}

void settings::setSoundfile(QString Soundfile)
{
    ui->lineEdit_soundfile->setText(Soundfile);
}
