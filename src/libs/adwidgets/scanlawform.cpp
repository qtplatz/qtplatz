#include "scanlawform.hpp"
#include "ui_scanlawform.h"

using namespace adwidgets;

ScanLawForm::ScanLawForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScanLawForm)
{
    ui->setupUi(this);
}

ScanLawForm::~ScanLawForm()
{
    delete ui;
}

void
ScanLawForm::setLength( double value, bool variable )
{
    ui->doubleSpinBox->setValue( value );
}

void
ScanLawForm::setAcceleratorVoltage( double value, bool variable )
{
    ui->doubleSpinBox_2->setValue( value );
}

void
ScanLawForm::setTDelay( double value, bool variable )
{
    ui->doubleSpinBox_3->setValue( value );
}
        
double
ScanLawForm::length() const
{
    return ui->doubleSpinBox->value();
}

double
ScanLawForm::acceleratorVoltage() const
{
    return ui->doubleSpinBox_2->value();
}

double
ScanLawForm::tDelay() const
{
    return ui->doubleSpinBox_3->value();
}

