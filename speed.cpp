#include "speed.h"
#include "ui_speed.h"

speed::speed(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::speed)
{
    ui->setupUi(this);
    connect(ui->horizontalSliderSpeed,SIGNAL(sliderMoved(int)),this,SLOT(changeSpeed(int)));
    connect(ui->horizontalSliderSpeed,SIGNAL(valueChanged(int)),this,SLOT(displaySpeed(int)));
}

speed::~speed()
{
    delete ui;
}

void speed::setSpeed(int Speed)
{
    ui->horizontalSliderSpeed->setValue(Speed);
    displaySpeed(Speed); // make sure it is displayed initially even if the value did not change
}

int speed::getSpeed(void)
{
    return ui->horizontalSliderSpeed->value();
}

void speed::changeSpeed(int Speed)
{
    libvlc_media_player_set_rate(vlcPlayer,((float)Speed)/100.0); // change playback speed
}

void speed::displaySpeed(int Speed)
{
    ui->labelSpeed->setText(QString("speed %1\%").arg(Speed));
}
