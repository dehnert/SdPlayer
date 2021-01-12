#include "breaktimer.h"
#include "ui_breaktimer.h"

Breaktimer::Breaktimer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Breaktimer)
{
    ui->setupUi(this);
    connect(ui->horizontalSlider_time, SIGNAL(valueChanged(int)), this, SLOT(sliderchanged(int)));
    connect(ui->doubleSpinBox_time, SIGNAL(valueChanged(double)), this, SLOT(spinboxchanged(double)));
}

Breaktimer::~Breaktimer()
{
    delete ui;
}

void Breaktimer::sliderchanged(int val)
{
    ui->doubleSpinBox_time->setValue(((double)val)/10.0);
}

void Breaktimer::spinboxchanged(double val)
{
    ui->horizontalSlider_time->setValue((int)(val*10.0+.01));
}

void Breaktimer::setTime(int time)
{
    ui->horizontalSlider_time->setValue(time);
}

int Breaktimer::getTime(void)
{
    return ui->horizontalSlider_time->value();
}
