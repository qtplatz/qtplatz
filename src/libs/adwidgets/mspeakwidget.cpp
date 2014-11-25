/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mspeakwidget.hpp"
#include "mspeaksummary.hpp"
#include "toftable.hpp"
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adportable/float.hpp>
#include <adportable/polfit.hpp>
#include <adportable/is_type.hpp>
#include <QSplitter>
#include <QBoxLayout>
#include <tuple>

using namespace adwidgets;
using namespace adcontrols::metric;

MSPeakWidget::~MSPeakWidget()
{
}

MSPeakWidget::MSPeakWidget(QWidget *parent) : QWidget(parent)
                                        , peakSummary_( new MSPeakSummary(this) )
                                        , tofTable_( new TOFTable(this) )
                                        , mspeaks_( new adcontrols::MSPeaks )
{
    if ( QSplitter * splitter = new QSplitter ) {

        splitter->addWidget( peakSummary_.get() );
        splitter->addWidget( tofTable_.get() );
        splitter->setOrientation( Qt::Horizontal );

        QVBoxLayout * layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 2 );
        layout->addWidget( splitter );
    }
}

void *
MSPeakWidget::query_interface_workaround( const char * typenam )
{
    if ( typenam == typeid( MSPeakWidget ).name() )
        return static_cast< MSPeakWidget * >(this);
    return 0;
}

void
MSPeakWidget::OnCreate( const adportable::Configuration& )
{
}

void
MSPeakWidget::OnInitialUpdate()
{
    peakSummary_->onInitialUpdate( this );
    tofTable_->onInitialUpdate();
}

void
MSPeakWidget::onUpdate( boost::any& )
{
}

void
MSPeakWidget::OnFinalClose()
{
    for ( auto& w: clients_ ) {
        disconnect(this, SIGNAL(onSetData( const QString&, const adcontrols::MSPeaks&)), w, SLOT(handleSetData(const QString&, const adcontrols::MSPeaks&)) );
        disconnect(this, SIGNAL(onSetData( int, const adcontrols::MSPeaks& ) ), w, SLOT(handleSetData( int, const adcontrols::MSPeaks& )) );        
    }
}

bool
MSPeakWidget::getContents( boost::any& ) const
{
    return false;
}

bool
MSPeakWidget::setContents( boost::any& a )
{
    if ( adportable::a_type< QWidget * >::is_a( a ) ) {
        QWidget * w = boost::any_cast< QWidget * >( a );
        if ( std::find_if( clients_.begin(), clients_.end(), [=]( const QWidget * a ){ return a == w; }) == clients_.end() ) {
            bool res1, res2;
            res1 = connect(this, SIGNAL(onSetData( const QString&, const adcontrols::MSPeaks&)), w, SLOT(handleSetData(const QString&, const adcontrols::MSPeaks&)) );
            res2 = connect(this, SIGNAL(onSetData( int, const adcontrols::MSPeaks& ) ), w, SLOT(handleSetData( int, const adcontrols::MSPeaks& )) );
            if ( res1 || res2 )
                clients_.push_back( w );
        }
    }
    return false;
}

void
MSPeakWidget::handle_add_mspeaks( const adcontrols::MSPeaks& peaks )
{
    for ( auto& peak: peaks ) {
        auto it = std::find_if( mspeaks_->begin(), mspeaks_->end(), [=]( const adcontrols::MSPeak& a ){
                return
                adportable::compare<double>::essentiallyEqual(a.time(), peak.time()) &&
                adportable::compare<double>::essentiallyEqual(a.mass(), peak.mass());
            });
        if ( it == mspeaks_->end() ) {
            (*mspeaks_) << peak;
            tofTable_->addPeak( peak );
        }
    }

    std::vector< std::tuple< double, double, double > > slopes;
    // estimate t-delay by ions
    do {
        std::map< int, adcontrols::MSPeaks > d;
        for ( auto& peak: *mspeaks_ )
            d[ peak.mode() ] << peak;
        for ( auto& t: d ) {
            const adcontrols::MSPeaks& pks = t.second;
            const double length = pks[0].flight_length();
            if ( pks.size() >= 2 ) {
                std::vector<double> x, y, coeffs;
                for ( auto& pk: pks ) {
                    x.push_back( std::sqrt( pk.mass() ) );
                    y.push_back( scale_to_micro( pk.time() ) );
                }

                adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ); // sqrt(m), time(us) for each length

                double t1 = length / adportable::polfit::estimate_y( coeffs, 1.0 ); // time for unit sqrt(m)

				peakSummary_->setPolynomials( t.first
                                              , coeffs
                                              , adportable::polfit::standard_error( x.data(), y.data(), x.size(), coeffs )
                                              , t1
                                              , length );
                slopes.push_back( std::make_tuple( pks[0].flight_length(), coeffs[0], coeffs[1] ) );
            }
        }
    } while(0);
    
    // estimate t-delay by flength
    do {
        std::map< std::string, adcontrols::MSPeaks > d;
        for ( auto& peak: *mspeaks_ ) {
            if ( ! peak.formula().empty() )
                d[ peak.formula() ] << peak;
        }

        for ( auto& t: d ) {
            const adcontrols::MSPeaks& pks = t.second;
            if ( pks.size() >= 2 ) {
                std::vector<double> x, y, coeffs;
                // length, time plot
                for ( auto& pk: pks ) {
                    x.push_back( pk.flight_length() );
                    y.push_back( scale_to_micro( pk.time() ) );
                }
                adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ); // L(m), time(us) for each formula
                double v = std::sqrt( pks[0].mass() ) / adportable::polfit::estimate_y( coeffs, 1.0 );  // time for 1.0mL := velocity
                peakSummary_->setPolynomials( t.first, coeffs, adportable::polfit::standard_error( x.data(), y.data(), x.size(), coeffs ), v );
            }
        }
    } while(0);

    // estimate overall calibration
    if ( slopes.size() >= 2 ) {
        std::vector<double> x, y0, y1, coeffs0, coeffs1; 
        for ( auto& item: slopes ) {
            x.push_back( std::get<0>(item) ); // length
            y0.push_back( std::get<1>(item) ); // intercept (a)
            y1.push_back( std::get<2>(item) ); // slope (b)
        }
        adportable::polfit::fit( x.data(), y0.data(), x.size(), 2, coeffs0 );
        adportable::polfit::fit( x.data(), y1.data(), x.size(), 2, coeffs1 );
        peakSummary_->setResult( 0, coeffs0, adportable::polfit::standard_error( x.data(), y0.data(), x.size(), coeffs0 ) );
        peakSummary_->setResult( 1, coeffs1, adportable::polfit::standard_error( x.data(), y1.data(), x.size(), coeffs1 ) );
    }
}

void
MSPeakWidget::currentChanged( int mode )
{
    adcontrols::MSPeaks peaks;
    
    for ( auto& peak: *mspeaks_ ) {
        if ( peak.mode() == mode )
            peaks << peak;
    }
    if ( peaks.size() >= 2 ) {
        std::vector<double> x, y, coeffs;
        for ( auto& pk: peaks ) {
            x.push_back( scale_to_micro( pk.time() ) );
            y.push_back( std::sqrt( pk.mass() ) );
        }
        adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs );
        peaks.polinomials( x, y, coeffs );
        emit onSetData( mode, peaks );
    }
}

void
MSPeakWidget::currentChanged( const std::string& formula )
{
    adcontrols::MSPeaks peaks;
    
    for ( auto& peak: *mspeaks_ ) {
        if ( ! peak.formula().empty() && peak.formula() == formula )
            peaks << peak;
    }
    if ( peaks.size() >= 2 ) {
        std::vector<double> x, y, coeffs;
        for ( auto& pk: peaks ) {
			x.push_back( pk.flight_length() );
            y.push_back( scale_to_micro( pk.time() ) );
        }
        adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs );
        peaks.polinomials( x, y, coeffs );
        emit onSetData( QString::fromStdString( formula ), peaks );
    }
}
