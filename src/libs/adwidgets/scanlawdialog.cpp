/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adcontrols/metric/prefix.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <workaround/boost/archive/xml_woarchive.hpp>
#include <workaround/boost/archive/xml_wiarchive.hpp>
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QSignalBlocker>
#include <cmath>
#include <sstream>

namespace adwidgets {

    class ScanLawDialog::impl : public adcontrols::ScanLaw {
    public:

        std::unique_ptr< adportable::TimeSquaredScanLaw > tof_;
        std::weak_ptr< const adcontrols::MassSpectrometer > spectrometer_;

        impl() {
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
            return tof_->getMass( secs, mode );
        }
        
        double getTime( double mass, int mode ) const override {
            return tof_->getTime( mass, mode );
        }
        
        double getMass( double secs, double fLength ) const override {
            return tof_->getMass( secs, fLength );
        }
        
        double getTime( double mass, double fLength ) const override {
            return tof_->getTime( mass, fLength );
        }
        
        double fLength( int mode ) const override {
            return tof_->fLength( mode );
        }

        std::vector< std::pair< double, double > > time_mass_array_;

    };

    class ScanLaw_archive {
    public:
        double acceleratorVoltage;
        double tDelay;

        ScanLaw_archive() : acceleratorVoltage( 0 ), tDelay( 0 ) {}
        ScanLaw_archive( double a, double t ) : acceleratorVoltage( a ), tDelay( t ) {}

        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( acceleratorVoltage );
            ar & BOOST_SERIALIZATION_NVP( tDelay );
        }
    };
    
}

using namespace adwidgets;
using namespace adcontrols::metric;

ScanLawDialog::ScanLawDialog(QWidget *parent) : QDialog(parent)
                                              , ui( new Ui::ScanLawDialog )
                                              , impl_( new impl() )
{
    ui->setupUi(this);
    //ui->doubleSpinBox_4->setValue( 609.2 );

    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, &QDialog::customContextMenuRequested, [this]( const QPoint& pt ){
            QMenu menu;
            menu.addAction( tr( "Copy" ), this, SLOT( handleCopyToClipboard() ) );
            menu.addAction( tr( "Paste" ), this, SLOT( handlePaste() ) );
            menu.exec( mapToGlobal( pt ) );
        });

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

    // mode
    connect( ui->spinBox, static_cast< void(QSpinBox::*)(int) >(&QSpinBox::valueChanged)
             , [this] ( int mode ) {
                 if ( auto sp = impl_->spectrometer_.lock() ) {
                     if ( auto scanlaw = sp->scanLaw() ) {
                         double L = scanlaw->fLength( mode );
                         ui->doubleSpinBox->setValue( L );
                     }
                 }
             });

    ui->spinBox->setEnabled( false ); // mode is disabled until MassSpectrometer to be specified
    
    // ---------- calculator -----------
    // m/z
    connect( ui->doubleSpinBox_4, static_cast< void(QDoubleSpinBox::*)(double) >(&QDoubleSpinBox::valueChanged)
             , [this] ( double mass ) {
                 auto law = impl_->scanLaw( ui );
                 QSignalBlocker blocks [] = { QSignalBlocker( ui->doubleSpinBox_5 ), QSignalBlocker( ui->lineEdit ) };
                 (void)blocks;
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
                 (void)blocks;
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
ScanLawDialog::keyPressEvent( QKeyEvent * event )
{
	if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
		return;
	if ( event->matches( QKeySequence::Copy ) ) {
	    handleCopyToClipboard();
	} else if ( event->matches( QKeySequence::Paste ) ) {
		handlePaste();
	}
	QDialog::keyPressEvent( event );
}

void
ScanLawDialog::setScanLaw( const adcontrols::ScanLaw& law )
{
    double tDelay = law.getTime( 0, 0 );
    double t = law.getTime( 100, 0 );
    double m = law.getMass( t, 0 );
    double a = adportable::TimeSquaredScanLaw::acceleratorVoltage( m, t, law.fLength( 0 ), tDelay );

    setValues( law.fLength( 0 ), a, tDelay, 0 );
}

void
ScanLawDialog::setScanLaw( std::shared_ptr< const adcontrols::MassSpectrometer > sp, int mode )
{
    impl_->spectrometer_ = sp;

    if ( auto scanlaw = sp->scanLaw() ) {
    
        double tDelay = scanlaw->getTime( 0, mode );
        double t = scanlaw->getTime( 100, mode );
        double m = scanlaw->getMass( t, mode );
        double a = adportable::TimeSquaredScanLaw::acceleratorVoltage( m, t, scanlaw->fLength( mode ), tDelay );
        
        setValues( scanlaw->fLength( mode ), a, tDelay, mode );
        ui->spinBox->setEnabled( true );
    }
}

const adcontrols::ScanLaw&
ScanLawDialog::scanLaw() const
{
    double length = ui->doubleSpinBox->value();
    double vacc  = ui->doubleSpinBox_2->value();
    double tDelay = ui->doubleSpinBox_3->value() / 1.0e6;
    impl_->tof_.reset( new adportable::TimeSquaredScanLaw( vacc, tDelay, length ) );
    return *impl_;
}

void
ScanLawDialog::setCalculator()
{
    auto law = impl_->scanLaw( ui );
    // if compute time
    double mass = ui->doubleSpinBox_4->value();
    int mode = ui->spinBox->value();

    QSignalBlocker block5( ui->doubleSpinBox_5 );
    QSignalBlocker block6( ui->doubleSpinBox_6 );
    ui->doubleSpinBox_5->setValue( scale_to_micro( law.getTime( mass, mode ) ) ); // -> us
    ui->doubleSpinBox_6->setValue( law.acceleratorVoltage( mass, law.getTime( mass, mode ), law.fLength( mode ), law.tDelay() ) );
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
ScanLawDialog::setValues( double fLength, double accVoltage, double tDelay, int mode )
{
    QSignalBlocker blocks[] = { QSignalBlocker(ui->doubleSpinBox), QSignalBlocker(ui->doubleSpinBox_2)
                                , QSignalBlocker( ui->doubleSpinBox_3 ), QSignalBlocker( ui->doubleSpinBox_6 )
                                , QSignalBlocker( ui->spinBox ) };
    (void)blocks;

    ui->doubleSpinBox->setValue( fLength );
    ui->doubleSpinBox_2->setValue( accVoltage );
    ui->doubleSpinBox_3->setValue( tDelay * 1.0e6 );
    ui->doubleSpinBox_6->setValue( accVoltage );
    ui->spinBox->setValue( mode );

    impl_->tof_.reset( new adportable::TimeSquaredScanLaw( accVoltage, tDelay, fLength ) );

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

        int mode = ui->spinBox->value();
        double tDelay = scale_to_base( ui->doubleSpinBox_3->value(), micro );

        double va = adportable::TimeSquaredScanLaw::acceleratorVoltage( impl_->time_mass_array_[ 0 ].second  // mass
                                                                        , impl_->time_mass_array_[ 0 ].first // time
                                                                        , impl_->fLength( mode ) // flength
                                                                        , tDelay ); // tDelay
        ui->doubleSpinBox_2->setValue( va );
        // ui->doubleSpinBox_3->setValue( 0 );

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

void
ScanLawDialog::handleCopyToClipboard()
{
	QString selected_text;

    ScanLaw_archive x( acceleratorVoltage(), tDelay() );
    
	selected_text.append( QString::number( acceleratorVoltage(), 'e', 14 ) );
	selected_text.append( '\t' );
	selected_text.append( QString::number( tDelay(), 'e', 14 ) );

	QMimeData * md = new QMimeData();
	md->setText( selected_text );
    
    std::wostringstream os;
    try {
        boost::archive::xml_woarchive ar ( os );
        ar & boost::serialization::make_nvp( "scanlaw", x );
        QString xml( QString::fromStdWString( os.str() ) );
        md->setData( QLatin1String( "application/scanlaw-xml" ), xml.toUtf8() );
    } catch ( std::exception& ex ) {
        BOOST_THROW_EXCEPTION( ex );
    }

	QApplication::clipboard()->setMimeData( md );
}

void
ScanLawDialog::handlePaste()
{
    auto md = QApplication::clipboard()->mimeData();
    auto data = md->data( "application/scanlaw-xml" );
    if ( !data.isEmpty() ) {
        QString utf8( QString::fromUtf8( data ) );
        std::wistringstream is( utf8.toStdWString() );
        boost::archive::xml_wiarchive ar( is );
        try {
            ScanLaw_archive x;
            ar & boost::serialization::make_nvp( "scanlaw", x );
            int mode = ui->spinBox->value();
            if ( auto sp = impl_->spectrometer_.lock() )
                setValues( sp->scanLaw()->fLength( mode ), x.acceleratorVoltage, x.tDelay, mode );
            else
                setValues( impl_->tof_->fLength( mode ), x.acceleratorVoltage, x.tDelay, mode );
        } catch ( std::exception& ex ) {
            BOOST_THROW_EXCEPTION( ex );
        }
    }
}

