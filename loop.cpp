#include "loop.h"
#include "ui_loop.h"

Loop::Loop(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Loop)
{
    ui->setupUi(this);
    // connect spinboxes and sliders so changing either affects both (this is too easy!)
    connect(ui->spinBox_start, SIGNAL(valueChanged(int)), ui->horizontalSlider_start, SLOT(setValue(int)));
    connect(ui->horizontalSlider_start, SIGNAL(valueChanged(int)), ui->spinBox_start, SLOT(setValue(int)));
    connect(ui->spinBox_end, SIGNAL(valueChanged(int)), ui->horizontalSlider_end, SLOT(setValue(int)));
    connect(ui->horizontalSlider_end, SIGNAL(valueChanged(int)), ui->spinBox_end, SLOT(setValue(int)));
    connect(ui->pushButton_start, SIGNAL(clicked()), this, SLOT(setStart()));
    connect(ui->pushButton_end, SIGNAL(clicked()), this, SLOT(setEnd()));
}

Loop::~Loop()
{
    delete ui;
}

void Loop::changeEvent(QEvent *e)
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

void Loop::setTotalTime(int totalTime)
{
    ui->horizontalSlider_start->setMaximum(totalTime-1);
    ui->spinBox_start->setMaximum(totalTime-1);
    ui->horizontalSlider_end->setMaximum(totalTime-1);
    ui->spinBox_end->setMaximum(totalTime-1);
}

void Loop::setLoopStart(int loopStart)
{
    ui->spinBox_start->setValue(loopStart);
}

int Loop::getLoopStart(void)
{
    return ui->spinBox_start->value();
}

void Loop::setLoopEnd(int loopEnd)
{
    ui->spinBox_end->setValue(loopEnd);
}

int Loop::getLoopEnd(void)
{
    return ui->spinBox_end->value();
}

void Loop::setStart(void)
{
    ui->spinBox_start->setValue(zeroStart);
}

void Loop::setEnd(void)
{
    ui->spinBox_end->setValue(zeroEnd);
}
