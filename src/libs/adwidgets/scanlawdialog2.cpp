/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "scanlawdialog2.hpp"
#include "scanlawform.hpp"
#include "moltableview.hpp"
#include <adportable/timesquaredscanlaw.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adportable/debug.hpp>
#include <adportable/polfit.hpp>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMenu>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItemModel>
#include <ratio>

namespace adwidgets {

    class ScanLawDialog2::impl {
    public:
        enum columns { c_id, c_formula, c_mass, c_time, c_error };
        
        impl() : model_( std::make_unique< QStandardItemModel >() )  {
            model_->setColumnCount( 5 );
            model_->setHeaderData( c_id,      Qt::Horizontal, QObject::tr( "id" ) );
            model_->setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "Formula" ) );
            model_->setHeaderData( c_mass,    Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
            model_->setHeaderData( c_time,    Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
            model_->setHeaderData( c_error,   Qt::Horizontal, QObject::tr( "Error (mDa)" ) );
        }

        std::unique_ptr< QStandardItemModel > model_;
    };
    
};


using namespace adwidgets;

ScanLawDialog2::ScanLawDialog2(QWidget *parent) : QDialog(parent)
                                                , impl_( std::make_unique< impl >() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(4);
        layout->setSpacing(2);
        
        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new ScanLawForm ) ); 
            splitter->addWidget( ( new MolTableView ) );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 1 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }

        if ( auto table = findChild< MolTableView * >() ) {
            
            table->setModel( impl_->model_.get() );
            table->setColumnHidden( impl::c_id, true );
            table->setColumnField( impl::c_formula, ColumnState::f_formula, true, true ); // editable, checkable
            table->setColumnField( impl::c_mass, ColumnState::f_mass );
            table->setColumnField( impl::c_time, ColumnState::f_time );
            table->setPrecision( impl::c_time, 4 );

            table->onInitialUpdate();
        }

        if ( auto buttons = findChild< QDialogButtonBox * >() ) {

            connect( buttons->button( QDialogButtonBox::Apply ), &QPushButton::clicked, this, [&](){ QDialog::accept(); } );

            connect( buttons, &QDialogButtonBox::rejected, this, [&](){ QDialog::reject(); } );
        }

        if ( auto form = findChild< ScanLawForm * >() ) {
            form->setLength( 0.5 );
            form->setAcceleratorVoltage( 4000.0 );
            form->setTDelay( 0.0 );
            connect( form, &ScanLawForm::valueChanged, this, [this]( int id ){
                    switch( id ) {
                    case 0: handleLengthChanged(); break;
                    case 1: handleAcceleratorVoltageChanged(); break;
                    case 2: handleTDelayChanged(); break;
                    }
                });
        }

    }
    resize( 600, 300 );
}

ScanLawDialog2::~ScanLawDialog2()
{
}

void
ScanLawDialog2::setLength( double value, bool variable )
{
    if ( auto form = findChild< ScanLawForm * >() ) {
        form->setLength( value );
    }
}

void
ScanLawDialog2::setAcceleratorVoltage( double value, bool variable )
{
    if ( auto form = findChild< ScanLawForm * >() ) {
        form->setAcceleratorVoltage( value );
    }    
}

void
ScanLawDialog2::setTDelay( double value, bool variable )
{
    if ( auto form = findChild< ScanLawForm * >() ) {
        form->setTDelay( value );
    }    
}

double
ScanLawDialog2::length() const
{
    if ( auto form = findChild< ScanLawForm * >() )
        return form->length();
    return 0;
}

double
ScanLawDialog2::acceleratorVoltage() const
{
    if ( auto form = findChild< ScanLawForm * >() )
        return form->acceleratorVoltage();
    return 0;    
}

double
ScanLawDialog2::tDelay() const
{
    if ( auto form = findChild< ScanLawForm * >() )    
        return form->tDelay();
    return 0;
}

void
ScanLawDialog2::addPeak( uint32_t id, const QString& formula, double time, double matchedMass )
{
    auto row = impl_->model_->rowCount();
    auto& model = *impl_->model_;

    model.setRowCount( row + 1 );
    model.setData( model.index( row, impl::c_id ), id, Qt::EditRole );
    model.setData( model.index( row, impl::c_formula ), formula, Qt::EditRole );
    double exact_mass = MolTableView::getMonoIsotopicMass( formula, "" );
    model.setData( model.index( row, impl::c_mass), exact_mass, Qt::EditRole );
    model.setData( model.index( row, impl::c_time), time * std::micro::den, Qt::EditRole );
    model.setData( model.index( row, impl::c_error), ( exact_mass - matchedMass ) * std::milli::den, Qt::EditRole );
    
    if ( auto item = model.item( row, impl::c_formula ) ) {
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        item->setEditable( true );
        model.setData( model.index( row, impl::c_formula ), Qt::Checked, Qt::CheckStateRole );
    }
}

void
ScanLawDialog2::updateMassError()
{
    auto& model = *impl_->model_;    

    if ( auto form = findChild< ScanLawForm * >() ) {
        adportable::TimeSquaredScanLaw scanlaw( form->acceleratorVoltage()
                                                , form->tDelay() / std::micro::den, form->length() );

        for ( int row = 0; row < model.rowCount(); ++row ) {
            double time = model.index( row, impl::c_time ).data( Qt::EditRole ).toDouble() / std::micro::den;
            double exact_mass = model.index( row, impl::c_mass ).data( Qt::EditRole ).toDouble();
            double mass = scanlaw.getMass( time, 0 );
            model.setData( model.index( row, impl::c_error), ( exact_mass - mass ) * std::milli::den, Qt::EditRole );
        }
    }
}

bool
ScanLawDialog2::commit()
{
    adcontrols::MSPeaks peaks;

    if ( read( peaks ) ) {

        if ( auto form = findChild< ScanLawForm * >() ) {

            double t0, acclVolts;

            if ( estimateAcceleratorVoltage( t0, acclVolts, peaks ) ) {
                form->setAcceleratorVoltage( acclVolts );
                form->setTDelay( t0 * std::micro::den );
                
                updateMassError();
                return true;
            }
        }
    }
    return false;
}

void
ScanLawDialog2::handleLengthChanged()
{
    adcontrols::MSPeaks peaks;

    if ( read( peaks ) ) {
    
        if ( auto form = findChild< ScanLawForm * >() ) {
            
            double t0, acclVolts;
            
            if ( estimateAcceleratorVoltage( t0, acclVolts, peaks ) ) {
                form->setAcceleratorVoltage( acclVolts );
                form->setTDelay( t0 * std::micro::den );
                
                updateMassError();                
            }
        }
    }
}

void
ScanLawDialog2::handleAcceleratorVoltageChanged()
{
    adcontrols::MSPeaks peaks;

    if ( read( peaks ) ) {
    
        if ( auto form = findChild< ScanLawForm * >() ) {
            
            double t0, L;
            
            if ( estimateLength( t0, L, peaks ) ) {
                form->setLength( L );
                form->setTDelay( t0 * std::micro::den );
                form->setLengthPrecision( 6 );
                updateMassError();
            }
        }
    }
}

void
ScanLawDialog2::handleTDelayChanged()
{
}

bool
ScanLawDialog2::read( adcontrols::MSPeaks& peaks ) const
{
    const auto& m = *impl_->model_;
    const size_t rowCount = m.rowCount();

    peaks.clear();
    
    for ( int row = 0; row < impl_->model_->rowCount(); ++row ) {
        
        auto formula = m.index( row, impl::c_formula ).data( Qt::EditRole ).toString();
        double exact_mass = m.index( row, impl::c_mass ).data( Qt::EditRole ).toDouble();
        
        if ( ! formula.isEmpty() && exact_mass > 0.5 ) {
            
            peaks << adcontrols::MSPeak( formula.toStdString()
                                         , 0.0 // mass (observed)
                                         , m.index( row, impl::c_time ).data( Qt::EditRole ).toDouble() / std::micro::den // time
                                         , 0   // mode
                                         , m.index( row, impl::c_id ).data( Qt::EditRole ).toInt() // spectrum index
                                         , m.index( row, impl::c_mass ).data( Qt::EditRole ).toDouble() );
        }
        
    }

    return peaks.size();
}

bool
ScanLawDialog2::estimateAcceleratorVoltage( double& t0, double& v, const adcontrols::MSPeaks& peaks ) const
{
    const double L = findChild< ScanLawForm * >()->length();

    if ( peaks.size() < 1 )
        return false;
    
    if ( peaks.size() == 1 ) {
        t0 = 0.0;
        const adcontrols::MSPeak& pk = peaks[ 0 ];
        v = adportable::TimeSquaredScanLaw::acceleratorVoltage( pk.exact_mass(), pk.time(), L, t0 );
        return true;
        
    } else if ( peaks.size() >= 2 ) {
        
        std::vector<double> x, y, coeffs;
        
        for ( auto& pk : peaks ) {
            x.push_back( std::sqrt( pk.exact_mass() ) * L );
            y.push_back( pk.time() );
        }
        
        if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) {
            
            t0 = coeffs[ 0 ];
            double t1 = adportable::polfit::estimate_y( coeffs, 1.0 ); // estimate tof for m/z = 1.0, 1mL
            v = adportable::TimeSquaredScanLaw::acceleratorVoltage( 1.0, t1, 1.0, t0 );

            return true;
        }
    }
    return false;
}

bool
ScanLawDialog2::estimateLength( double& t0, double& L, const adcontrols::MSPeaks& peaks ) const
{
    if ( peaks.size() < 1 )
        return false;

    const double V = findChild< ScanLawForm * >()->acceleratorVoltage();
    
    if ( peaks.size() == 1 ) {
        t0 = 0.0;
        const adcontrols::MSPeak& pk = peaks[ 0 ];
        L = sqrt( ( adportable::kTimeSquaredCoeffs * V * pk.time() * pk.time() ) / pk.exact_mass() );
        return true;
        
    } else if ( peaks.size() >= 2 ) {
        
        std::vector<double> x, y, coeffs;
        
        for ( auto& pk : peaks ) {
            x.push_back( std::sqrt( pk.exact_mass() ) ); // assume L = 1.0m
            y.push_back( pk.time() );
        }
        
        if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) {
            
            t0 = coeffs[ 0 ];
            double t1 = adportable::polfit::estimate_y( coeffs, 1.0 );   // estimate tof for m/z = 1.0, for 1m

            double t = t1 - t0;
            L = sqrt( V * adportable::kTimeSquaredCoeffs * ( t * t ) );

            return true;
        }

    }
    return false;

}

//////////////
