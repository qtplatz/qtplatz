/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "scanlawdialog.hpp"
#include "ui_scanlawdialog.h"
#include <adportable/polfit.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <QSignalBlocker>
#include <cmath>

namespace adwidgets {

    class ScanLawDialog::impl : public adcontrols::ScanLaw {
    public:
        std::unique_ptr< adportable::TimeSquaredScanLaw > law_;
        impl() : law_( new adportable::TimeSquaredScanLaw() ) {
        }
        
        ~impl() {
        }

        adportable::TimeSquaredScanLaw scanLaw( adwidgets::Ui::ScanLawDialog * ui ) {
            double flength = ui->doubleSpinBox->value();
            double vacc = ui->doubleSpinBox_2->value();
            double tDelay = ui->doubleSpinBox_3->value() * 1.0e-6;
            return adportable::TimeSquaredScanLaw( vacc, tDelay, flength );
        }
        
        // adcontrols::ScanLaw
        double getMass( double secs, int mode ) const override {
            return law_->getMass( secs, mode );
        }
        double getTime( double mass, int mode ) const override {
            return law_->getTime( mass, mode );
        }
        double getMass( double secs, double fLength ) const override {
            return law_->getMass( secs, fLength );
        }
        double getTime( double mass, double fLength ) const override {
            return law_->getTime( mass, fLength );
        }
        double fLength( int mode ) const override {
            return law_->fLength( mode );
        }

        std::vector< std::pair< double, double > > time_mass_array_;
    };
}

using namespace adwidgets;

ScanLawDialog::ScanLawDialog(QWidget *parent) : QDialog(parent)
                                              , ui( new Ui::ScanLawDialog )
                                              , impl_( new impl() )
{
    ui->setupUi(this);
    ui->doubleSpinBox_4->setValue( 609.2 );

    // flength
    connect( ui->doubleSpinBox, static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged)
             , [this] ( double flength ) {
                 setCalculator();
                 estimate();
             });
    // vacc
    connect( ui->doubleSpinBox_2, static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged)
             , [this] ( double vacc ) {
                 setCalculator();                 
             });    
    // tDelay
    connect( ui->doubleSpinBox_3, static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged)
             , [this] ( double tDelay ) {
                 setCalculator();                 
             });    

    // ---------- calculator -----------
    // m/z
    connect( ui->doubleSpinBox_4, static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged)
             , [this] ( double mass ) {
                 auto law = impl_->scanLaw( ui );
                 QSignalBlocker blocks [] = { QSignalBlocker( ui->doubleSpinBox_5 ), QSignalBlocker( ui->lineEdit ) };
                 ui->lineEdit->setText( QString() );  // clear formula
                 if ( ui->checkBox->isChecked() ) {
                     double tof = ui->doubleSpinBox_5->value() / 1.0e6;
                     ui->doubleSpinBox_6->setValue( law.acceleratorVoltage( mass, tof, law.fLength( 0 ), law.tDelay() ) );
                 } else
                     ui->doubleSpinBox_5->setValue( law.getTime( mass, 0 ) * 1.0e6 );
             });
    // tof
    connect( ui->doubleSpinBox_5, static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged)
             , [this] ( double tof ) {
                 tof /= 1.0e6;
                 QSignalBlocker blocks [] = { QSignalBlocker( ui->doubleSpinBox_4 ), QSignalBlocker( ui->lineEdit ) };
                 auto law = impl_->scanLaw( ui );
                 if ( ui->checkBox->isChecked() ) {
                     double mass = ui->doubleSpinBox_4->value();
                     ui->doubleSpinBox_6->setValue( law.acceleratorVoltage( mass, tof, law.fLength( 0 ), law.tDelay() ) );
                 } else {
                     ui->lineEdit->setText( QString() ); // clear formula
                     ui->doubleSpinBox_4->setValue( law.getMass( tof, 0 ) );
                 }
             });
    connect( ui->lineEdit, &QLineEdit::textEdited, [this] ( const QString& text ) {
                 adcontrols::ChemicalFormula c;
                 double mass = c.getMonoIsotopicMass( text.toStdString() );
                 if ( mass > 0.9 ) {
                     setMass( mass );
                 }
             } );
}

ScanLawDialog::~ScanLawDialog()
{
    delete ui;
    delete impl_;
}

void
ScanLawDialog::setScanLaw( const adcontrols::ScanLaw& law )
{
    double tDelay = law.getTime( 0, 0 );
    double t = law.getTime( 100, 0 );
    double m = law.getMass( t, 0 );
    double a = adportable::TimeSquaredScanLaw::acceleratorVoltage( m, t, law.fLength( 0 ), tDelay );

    setValues( law.fLength( 0 ), a, tDelay );
}

const adcontrols::ScanLaw&
ScanLawDialog::scanLaw() const
{
    double length = ui->doubleSpinBox->value();
    double vacc  = ui->doubleSpinBox_2->value();
    double tDelay = ui->doubleSpinBox_3->value() / 1.0e6;
    impl_->law_.reset( new adportable::TimeSquaredScanLaw( vacc, tDelay, length ) );
    return *impl_;
}

void
ScanLawDialog::setCalculator()
{
    auto law = impl_->scanLaw( ui );
    // if compute time
    double mass = ui->doubleSpinBox_4->value();

    QSignalBlocker block5( ui->doubleSpinBox_5 );
    QSignalBlocker block6( ui->doubleSpinBox_6 );
    ui->doubleSpinBox_5->setValue( law.getTime( mass, 0 ) * 1.0e6 ); // -> us
    ui->doubleSpinBox_6->setValue( law.acceleratorVoltage( mass, law.getTime( mass, 0 ), law.fLength( 0 ), law.tDelay() ) );
}

double
ScanLawDialog::fLength() const
{
    return ui->doubleSpinBox->value();
}

double
ScanLawDialog::acceleratorVoltage() const
{
    return ui->doubleSpinBox_2->value();
}

double
ScanLawDialog::tDelay() const
{
    return ui->doubleSpinBox_3->value() * 1.0e-6;  // seconds
}

void
ScanLawDialog::setValues( double fLength, double accVoltage, double tDelay )
{
    QSignalBlocker blocks[] = { QSignalBlocker(ui->doubleSpinBox), QSignalBlocker(ui->doubleSpinBox_2)
                                , QSignalBlocker( ui->doubleSpinBox_3 ), QSignalBlocker( ui->doubleSpinBox_6 ) };

    ui->doubleSpinBox->setValue( fLength );
    ui->doubleSpinBox_2->setValue( accVoltage );
    ui->doubleSpinBox_3->setValue( tDelay * 1.0e6 );
    ui->doubleSpinBox_6->setValue( accVoltage );

    impl_->law_.reset( new adportable::TimeSquaredScanLaw( accVoltage, tDelay, fLength ) );

    setCalculator();
}

double 
ScanLawDialog::mass() const
{
    return ui->doubleSpinBox_4->value();
}

void
ScanLawDialog::setMass( double mass )
{
    QSignalBlocker block( ui->doubleSpinBox_4 );
    ui->doubleSpinBox_4->setValue( mass );
}

QString
ScanLawDialog::formula() const
{
    return ui->lineEdit->text();
}

void
ScanLawDialog::setFormula( const QString& formula )
{
    QSignalBlocker block( ui->lineEdit );
    ui->lineEdit->setText( formula );
    setMass( adcontrols::ChemicalFormula().getMonoIsotopicMass( formula.toStdString() ) );
}

void
ScanLawDialog::setData( const std::vector< std::pair<double, double> >& time_mass_array )
{
    impl_->time_mass_array_ = time_mass_array;
    estimate();
}

void
ScanLawDialog::estimate()
{
    if ( impl_->time_mass_array_.empty() ) {
        return;

    } else if ( impl_->time_mass_array_.size() == 1 ) {

        double va = adportable::TimeSquaredScanLaw::acceleratorVoltage( impl_->time_mass_array_[ 0 ].second, impl_->time_mass_array_[ 0 ].first, impl_->fLength( 0 ), 0 );
        ui->doubleSpinBox_2->setValue( va );    
        ui->doubleSpinBox_3->setValue( 0 );

    } else {
        std::vector<double> x, y, coeffs;
        for ( auto& xy : impl_->time_mass_array_ ) {
            double mass = xy.second;
            double time = xy.first;
            x.push_back( std::sqrt( mass ) * impl_->fLength( 0 ) );
            y.push_back( time );
        }
        if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) {
            double t0 = coeffs[ 0 ];
            double t1 = adportable::polfit::estimate_y( coeffs, 1.0 );
            double va = adportable::TimeSquaredScanLaw::acceleratorVoltage( 1.0, t1, 1.0, t0 );

            ui->doubleSpinBox_2->setValue( va );
            ui->doubleSpinBox_3->setValue( t0 * 1.0e6 );
        }
    }
}
        
