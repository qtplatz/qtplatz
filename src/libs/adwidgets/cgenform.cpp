#include "cgenform.hpp"
#include "ui_cgenform.h"

using namespace adwidgets;

CGenForm::CGenForm(QWidget *parent) : QWidget(parent)
                                    , ui(new Ui::CGenForm)
{
    ui->setupUi(this);
    ui->doubleSpinBox->setMaximum( 1000.0 );
}

CGenForm::~CGenForm()
{
    delete ui;
}

void
CGenForm::setMassWidth( double width )
{
    ui->doubleSpinBox->setValue( width * std::milli::den );
}

void
CGenForm::setTimeWidth( double width )
{
    ui->doubleSpinBox_2->setValue( width * std::micro::den );
}

double
CGenForm::massWidth() const
{
    return ui->doubleSpinBox->value() / std::milli::den;
}

double
CGenForm::timeWidth() const
{
    return ui->doubleSpinBox_2->value() / std::micro::den;
}

bool
CGenForm::enableTime() const
{
    return ui->radioButton_2->isChecked();
}

void
CGenForm::setEnableTime( bool enable )
{
    ui->radioButton_2->setChecked( enable );
}

void
CGenForm::on_doubleSpinBox_valueChanged( double arg1 )
{
    emit valueChanged( ID_MASS_WIDTH, arg1 );
}

void
CGenForm::on_doubleSpinBox_2_valueChanged( double arg1 )
{
    emit valueChanged( ID_TIME_WIDTH, arg1 );
}

void
CGenForm::on_radioButton_2_toggled( bool checked )
{
    emit enableTimeChanged( checked );
}

void
CGenForm::setLabel( const QString& label )
{
    ui->label->setText( label );
}
