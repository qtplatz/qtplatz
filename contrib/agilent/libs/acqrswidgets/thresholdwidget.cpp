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

#include "thresholdwidget.hpp"
#include <acqrscontrols/u5303a/method.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/timedigitalmethod.hpp>
#include <adportable/is_type.hpp>
#include <adportable/serializer.hpp>
#include <adwidgets/findslopeform.hpp>
#include <adwidgets/thresholdactionform.hpp>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QPair>
#include <boost/exception/all.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/type_traits.hpp>
#include <type_traits>
#include <memory>

using namespace acqrswidgets;

ThresholdWidget::ThresholdWidget( const QString&
                                  , uint32_t nChannels
                                  , QWidget *parent) : QWidget(parent)
                                                     , nChannels_( nChannels )
{
    // Software TDC (Slope Time Digital Converter) UI
    if ( auto layout = new QVBoxLayout( this ) ) {

        if ( auto tab = new QTabWidget() ) {

            layout->addWidget( tab );

            for ( uint32_t channel = 0; channel < nChannels_; ++channel ) {
                auto title = QString( "CH%1" ).arg( QString::number( channel + 1 ) );
                auto ch = new adwidgets::findSlopeForm();
                ch->setTitle( channel, title );
                ch->setObjectName( title );
                tab->addTab( ch, title );
                // enable|disable
                connect( ch, &adwidgets::findSlopeForm::valueChanged, [this] ( int ch ) { emit valueChanged( idSlopeTimeConverter, ch ); } );
            }
            
            ///////////// Threshold action --- See also ap240form class /////////

            auto form = new adwidgets::ThresholdActionForm();
            form->setObjectName( "ThresholdWidget::ThresholdActionForm" );
            tab->addTab( form, tr( "Action" ) );
            connect( form, &adwidgets::ThresholdActionForm::valueChanged, [this](){ emit valueChanged( idThresholdAction, 0 ); } );
        }
    }
    
}

ThresholdWidget::~ThresholdWidget()
{
}

// LifeCycle
void
ThresholdWidget::OnCreate( const adportable::Configuration& )
{
}

void
ThresholdWidget::OnInitialUpdate()
{
}

void
ThresholdWidget::OnFinalClose()
{
}

bool
ThresholdWidget::getContents( boost::any& a ) const
{
    acqrscontrols::u5303a::method m;

    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {
        adcontrols::ControlMethodPtr ptr = boost::any_cast<adcontrols::ControlMethodPtr>(a);        
        adcontrols::TimeDigitalMethod tdm;
        get( tdm );
        ptr->append( tdm );
        
    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_pointer( a ) ) {
        // time function
        //assert( 0 );  // TBA
        return true;

    } else if ( adportable::a_type< acqrscontrols::u5303a::method >::is_pointer( a ) ) {

        auto pm = boost::any_cast<acqrscontrols::u5303a::method *>( a );
        //get( *pm );
        assert( 0 );
        return true;

    }
    return false;
}

bool
ThresholdWidget::setContents( boost::any&& a )
{
    const char * type = a.type().name();

    auto pi = adcontrols::ControlMethod::any_cast<>( )( a, adcontrols::TimeDigitalMethod::clsid() );
    if ( pi ) {
        adcontrols::TimeDigitalMethod tdm;
        if ( pi->get<>( *pi, tdm ) ) {
            set( tdm );
            return true;
        }
    }
    return false;
}

void
ThresholdWidget::onInitialUpdate()
{
}

void
ThresholdWidget::onStatus( int )
{
}

void
ThresholdWidget::get( adcontrols::TimeDigitalMethod& m ) const
{
    m.thresholds().clear();

    for ( uint32_t ch = 0; ch < nChannels_; ++ch ) {
        if ( auto form = findChild< adwidgets::findSlopeForm * >( QString( "CH%1" ).arg( char( '1' + ch ) ) ) ) {
            adcontrols::threshold_method t;
            form->get( t );
            m.thresholds().push_back( t );
        }
    }

    if ( auto form = findChild< adwidgets::ThresholdActionForm *>() ) {
        adcontrols::threshold_action a;
        form->get( a );
        m.action() = a;
    }
}

void
ThresholdWidget::set( const adcontrols::TimeDigitalMethod& m )
{
    for ( uint32_t ch = 0; ch < nChannels_; ++ch ) {
        if ( m.thresholds().size() > ch ) {
            if ( auto form = findChild< adwidgets::findSlopeForm * >( QString( "CH%1" ).arg( char( '1' + ch ) ) ) ) {
                form->set( m.thresholds()[ ch ] );
            }
        }
    }
    
    if ( auto form = findChild< adwidgets::ThresholdActionForm *>() ) {
        form->set( m.action() );
    }
}

void
ThresholdWidget::get( int ch, adcontrols::threshold_method& m ) const
{
    const QString name = QString( "CH%1" ).arg( QString::number( ch + 1 ) );

    if ( auto form = findChild< adwidgets::findSlopeForm * >( name ) ) {
        form->get( m );
    }
}

void
ThresholdWidget::set( int ch, const adcontrols::threshold_method& m )
{
    const QString name = QString( "CH%1" ).arg( QString::number( ch + 1 ) );

    if ( auto form = findChild< adwidgets::findSlopeForm * >( name ) ) {
        form->set( m );
    }
}

void
ThresholdWidget::get( adcontrols::threshold_action& m ) const
{
    if ( auto form = findChild< adwidgets::ThresholdActionForm *>() )
        form->get( m );
}

void
ThresholdWidget::set( const adcontrols::threshold_action& m )
{
    if ( auto form = findChild< adwidgets::ThresholdActionForm *>() )
        form->set( m );
}

void
ThresholdWidget::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > sp )
{
    if ( auto form = findChild< adwidgets::ThresholdActionForm *>() )
        form->setMassSpectrometer( sp );
}
