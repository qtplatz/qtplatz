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
#include <memory>

using namespace acqrswidgets;

ThresholdWidget::ThresholdWidget( const QString& model
                                  , uint32_t nChannels
                                  , QWidget *parent) : QWidget(parent)
                                                     , modelClass_( model )
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
        if ( modelClass_ == acqrscontrols::u5303a::method::modelClass() ) {
            auto it = ptr->find( ptr->begin(), ptr->end(), acqrscontrols::u5303a::method::modelClass() );

            if ( it != ptr->end() )
                it->get<>( *it, m );

            get( 0, m.threshold_ );
            ptr->append( m );

            return true;
        }
        // TBA: ap240 code and time event code
        
    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_pointer( a ) ) {
        // time function
        assert(0); // tba
#if 0  
        auto pi = boost::any_cast<adcontrols::ControlMethod::MethodItem *>( a );
        acqrscontrols::ap240::method m;
        get( m );
        pi->setModelname( "ap240" );
        pi->setItemLabel( "ap240" );
        pi->unitnumber( 1 );
        pi->funcid( 1 );
        pi->set<>( *pi, m ); // serialize
        return true;

    } else if ( adportable::a_type< acqrscontrols::ap240::method >::is_pointer( a ) ) {

        auto pm = boost::any_cast<acqrscontrols::ap240::method *>( a );
        get( *pm );
        return true;
#endif
    }
    return false;
}

bool
ThresholdWidget::setContents( boost::any& a )
{
    const char * type = a.type().name();

    std::shared_ptr< const adcontrols::ControlMethod::Method > ptr;

    if ( adportable::a_type < std::shared_ptr< const adcontrols::ControlMethod::Method > >::is_a( a ) ) {
        ptr = boost::any_cast <std::shared_ptr< const adcontrols::ControlMethod::Method>>( a );
    } else if ( adportable::a_type < std::shared_ptr< adcontrols::ControlMethod::Method > >::is_a( a ) ) {
        ptr = boost::any_cast <std::shared_ptr< adcontrols::ControlMethod::Method>>( a );
    }
    if ( ptr ) {
        auto it = ptr->find( ptr->begin(), ptr->end(), acqrscontrols::u5303a::method::modelClass() );
        if ( it != ptr->end() ) {
            acqrscontrols::u5303a::method m;
            if ( it->get<>( *it, m ) )
                set( 0, m.threshold_ );
        }
        return true;
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

#if 0
void
ThresholdWidget::set( const acqrscontrols::ap240::method& m )
{
    if ( auto gbox = findChild< QGroupBox * >( "CH-1" ) ) {
        QSignalBlocker block( gbox );
        gbox->setChecked( m.channels_ & 0x01 );
    }
    if ( auto gbox = findChild< QGroupBox * >( "CH-2" ) ) {
        QSignalBlocker block( gbox );
        gbox->setChecked( m.channels_ & 0x02 );
    }
    if ( auto gbox = findChild< QGroupBox * >( "Ext" ) ) {
        QSignalBlocker block( gbox );
        gbox->setChecked( m.trig_.trigPattern & 0x80000000 );
    }

    if ( auto form = findChild< ap240HorizontalForm *>() ) {
        QSignalBlocker block( form );
        form->set( m );
    }
    
    if ( auto form = findChild< ap240TriggerForm *>() ) {
        QSignalBlocker block( form );
        form->set( m );        
    }
    
    for ( auto form : findChildren< ap240VerticalForm * >() ) {
        QSignalBlocker block( form );
        form->set( m );        
    }
    set( 0, m.slope1_ );
    set( 1, m.slope2_ );
}

void
ThresholdWidget::get( acqrscontrols::ap240::method& m ) const
{
    uint32_t channels( 0 );
    
    if ( auto gbox = findChild< QGroupBox * >( "CH-1" ) ) {
        if ( gbox->isChecked() )
            channels |= 1;             
    }
    if ( auto gbox = findChild< QGroupBox * >( "CH-2" ) ) {
        if ( gbox->isChecked() )
            channels |= 2;
    }    
    m.channels_ = channels;
 
    if ( auto form = findChild< ap240HorizontalForm *>() ) {
        form->get( m );
    }
    
    if ( auto form = findChild< ap240TriggerForm *>() ) {
        form->get( m );        
    }
    
    for ( auto form : findChildren< ap240VerticalForm * >() ) {
        form->get( m );        
    }

    get( 0, m.slope1_ );
    get( 1, m.slope2_ );
}
#endif

void
ThresholdWidget::get( int ch, adcontrols::threshold_method& m ) const
{
    const QString name = QString( "CH%1" ).arg( QString::number(ch) );

    if ( auto form = findChild< adwidgets::findSlopeForm * >( name ) ) {
        form->get( m );
    }
}

void
ThresholdWidget::set( int ch, const adcontrols::threshold_method& m )
{
    const QString name = QString( "CH%1" ).arg( QString::number(ch) );    

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
