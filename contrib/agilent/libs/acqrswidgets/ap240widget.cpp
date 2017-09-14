/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "ap240widget.hpp"
#include "acqiriswidget.hpp"
#include <acqrscontrols/acqiris_method.hpp>
#include <acqrscontrols/ap240/method.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/threshold_action.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <adportable/serializer.hpp>
#include <adwidgets/thresholdactionform.hpp>
#include <adwidgets/findslopeform.hpp>
#include <qtwrapper/make_widget.hpp>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QModelIndex>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPair>
#include <boost/exception/all.hpp>

using namespace acqrswidgets;

ap240widget::ap240widget( QWidget *parent ) : QWidget( parent )
{
    if ( auto topLayout = new QHBoxLayout( this ) ) {
        topLayout->setSpacing( 0 );
        topLayout->setMargin( 0 );

        if ( auto gbox = qtwrapper::make_widget< QGroupBox >( "GroupBox", "AP240" ) ) {
            if ( auto layout = new QVBoxLayout( gbox ) ) {
                layout->setSpacing( 0 );
                layout->setMargin( 0 );
                if ( auto widget = new AcqirisWidget() ) {
                    layout->addWidget( widget );
                    widget->setStyleSheet( "QTreeView { background: #e8f4fc; }\n"
                                           "QTreeView::item:open { background-color: #1d3dec; color: white; }" );
                    connect( widget, &AcqirisWidget::dataChanged, [&]( const AcqirisWidget * w, int cat ){
                            emit valueChanged( idAP240Any, 0 );
                        });
                    connect( widget, &AcqirisWidget::stateChanged, [&]( const QModelIndex& index, bool ){
                            emit valueChanged( idChannels, index.row() - 2 );
                        });            
                }
            }
            topLayout->addWidget( gbox );
        }
    }
    
    set( std::make_shared< acqrscontrols::ap240::method >() );
}

ap240widget::~ap240widget()
{
}

// LifeCycle
void
ap240widget::OnCreate( const adportable::Configuration& )
{
}

void
ap240widget::OnInitialUpdate()
{
    if ( auto form = findChild< adwidgets::ThresholdActionForm * >() )
        form->OnInitialUpdate();

    // don't response to each key strokes on DoubleSpinBox
    for ( auto spin : findChildren< QDoubleSpinBox * >() )
        spin->setKeyboardTracking( false );
}

void
ap240widget::OnFinalClose()
{
}

bool
ap240widget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        adcontrols::ControlMethodPtr ptr = boost::any_cast<adcontrols::ControlMethodPtr>(a);        
        
        auto m = std::make_shared< acqrscontrols::ap240::method>();
        get( m );
        adcontrols::ControlMethod::MethodItem item( m->clsid(), m->modelClass() );
        item.setItemLabel( "ap240" );
        item.set<>( item, *m ); // serialize
        ptr->insert( item );
        
        return true;
        
    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_pointer( a ) ) {
        
        auto pi = boost::any_cast<adcontrols::ControlMethod::MethodItem *>( a );
        auto m = std::make_shared< acqrscontrols::ap240::method>();
        get( m );
        pi->setModelname( "ap240" );
        pi->setItemLabel( "ap240" );
        pi->unitnumber( 1 );
        pi->funcid( 1 );
        pi->set<>( *pi, *m ); // serialize
        return true;
    } else if ( adportable::a_type< acqrscontrols::ap240::method >::is_pointer( a ) ) {
        assert( 0 );
        // auto pm = boost::any_cast<acqrscontrols::ap240::method *>( a );
        // get( *pm );
        // return true;
    }
    return false;
}

bool
ap240widget::setContents( boost::any&& a )
{
    const adcontrols::ControlMethod::MethodItem * pi(0);

    if ( adportable::a_type < std::shared_ptr< const adcontrols::ControlMethod::Method > >::is_a( a ) ) {
        // adcontrols::ControlMethod::Method
        // find first one
        if ( auto ptr = boost::any_cast<std::shared_ptr< const adcontrols::ControlMethod::Method>>( a ) ) {
            auto it = ptr->find( ptr->begin(), ptr->end(), acqrscontrols::ap240::method::clsid() );
            if ( it != ptr->end() )
                pi = &( *it );
        }

    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_pointer( a ) ) {

        pi = boost::any_cast<const adcontrols::ControlMethod::MethodItem * >( a );             

    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_a( a ) ) {   

        pi = &boost::any_cast<const adcontrols::ControlMethod::MethodItem& >( a );
    }

    if ( pi ) {
        auto m = std::make_shared< acqrscontrols::ap240::method >();
		try {
            pi->get<>( *pi, *m );
            set( m );
		} catch (boost::exception& ex) {
			QMessageBox::warning(this, "AP240 Method", QString::fromStdString(boost::diagnostic_information(ex)));
		} catch ( ... ) {
			QMessageBox::warning(this, "AP240 Method", QString::fromStdString(boost::current_exception_diagnostic_information()));
		}

    }
    return true;
}

void
ap240widget::onInitialUpdate()
{
}

void
ap240widget::onStatus( int )
{
}

void
ap240widget::get( std::shared_ptr< acqrscontrols::ap240::method > m ) const
{
    if ( auto w = findChild< AcqirisWidget * >() ) {
        w->getContents( m );
        //get( 0, m->slope1_ );
        //get( 1, m->slope2_ );
        //get( m->action_ );
    }
}

void
ap240widget::set( std::shared_ptr< const acqrscontrols::ap240::method> m )
{
    if ( auto w = findChild< AcqirisWidget * >() ) {
        w->setContents( m );
        //set( 0, m->slope1_ );
        //set( 1, m->slope2_ );
        //set( m->action_ );
    }    
}
