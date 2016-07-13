/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <QDialogButtonBox>
#include <QSplitter>
#include <QBoxLayout>
#include <QMenu>
#include <QStandardItemModel>
#include <ratio>

namespace adwidgets {

    class ScanLawDialog2::impl {
    public:
        enum columns { c_id, c_formula, c_mass, c_time };
        
        impl() : model_( std::make_unique< QStandardItemModel >() )  {
            model_->setColumnCount( 4 );
            model_->setHeaderData( c_id,      Qt::Horizontal, QObject::tr( "id" ) );
            model_->setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "Formula" ) );
            model_->setHeaderData( c_mass,    Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
            model_->setHeaderData( c_time,    Qt::Horizontal, QObject::tr( "Time(&mu;s)" ) );
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
            connect( buttons, &QDialogButtonBox::accepted, this, [&](){ QDialog::accept(); } );
            connect( buttons, &QDialogButtonBox::rejected, this, [&](){ QDialog::reject(); } );
        }

        if ( auto form = findChild< ScanLawForm * >() ) {
            form->setLength( 0.5, false );
            form->setAcceleratorVoltage( 4000.0, true );
            form->setTDelay( 0.0, true );
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
        form->setLength( value, variable );
    }
}

void
ScanLawDialog2::setAcceleratorVoltage( double value, bool variable )
{
    if ( auto form = findChild< ScanLawForm * >() ) {
        form->setAcceleratorVoltage( value, variable );
    }    
}

void
ScanLawDialog2::setTDelay( double value, bool variable )
{
    if ( auto form = findChild< ScanLawForm * >() ) {
        form->setTDelay( value, variable );
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

    impl_->model_->setRowCount( row + 1 );
    impl_->model_->setData( impl_->model_->index( row, impl::c_id ), id, Qt::EditRole );
    impl_->model_->setData( impl_->model_->index( row, impl::c_formula ), formula, Qt::EditRole );
    impl_->model_->setData( impl_->model_->index( row, impl::c_mass), MolTableView::getMonoIsotopicMass( formula, "" ), Qt::EditRole );
    impl_->model_->setData( impl_->model_->index( row, impl::c_time), time * std::micro::den, Qt::EditRole );
}

bool
ScanLawDialog2::commit()
{
}

#if 0
bool
MSPeakWidget::estimateScanLaw( const adcontrols::MSPeaks& peaks, double& va, double& t0 )
{
    using namespace adcontrols::metric;
    
    infitof::ScanLaw law;

    if ( peaks.size() == 1 ) {

        const adcontrols::MSPeak& pk = peaks[ 0 ];
        va = law.acceleratorVoltage( pk.exact_mass(), pk.time(), pk.mode(), 0.0 );
        t0 = 0.0;
        return true;

    } else if ( peaks.size() >= 2 ) {
        
        std::vector<double> x, y, coeffs;

        for ( auto& pk : peaks ) {
            x.push_back( std::sqrt( pk.exact_mass() ) * law.fLength( pk.mode() ) );
            y.push_back( pk.time() );
        }

        if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) {

            t0 = coeffs[ 0 ];
            double t1 = adportable::polfit::estimate_y( coeffs, 1.0 ); // estimate tof for m/z = 1.0, 1mL
            va = adportable::TimeSquaredScanLaw::acceleratorVoltage( 1.0, t1, 1.0, t0 );

            return true;
        }
    }
    return false;
}
#endif

//////////////
