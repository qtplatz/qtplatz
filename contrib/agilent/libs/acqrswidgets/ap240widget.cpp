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
#include <QLabel>
#include <QLineEdit>
#include <QModelIndex>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPair>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>

using namespace acqrswidgets;

ap240widget::ap240widget( QWidget *parent ) : QWidget( parent )
{
    if ( auto topLayout = new QVBoxLayout( this ) ) {
        topLayout->setSpacing( 0 );
        topLayout->setMargin( 0 );

        if ( auto gbox = qtwrapper::make_widget< QGroupBox >( "GroupBox", "AP240" ) ) {
            gbox->setStyleSheet( "QGroupBox { margin-top: 2ex; margin-bottom: 0ex; margin-left: 0ex; margin-right: 0ex; }" );
            if ( auto layout = new QVBoxLayout( gbox ) ) {
                layout->setSpacing( 0 );
                layout->setMargin( 0 );
                if ( auto widget = new AcqirisWidget() ) {
                    layout->addWidget( widget );
                }
            }
            topLayout->addWidget( gbox );
        }
        if ( auto layout = new QHBoxLayout() ) {
            auto label = qtwrapper::make_widget< QLabel >( "label", "Ext. delays:" );
            layout->addWidget( label );
            layout->addWidget( qtwrapper::make_widget< QLineEdit >( "external_delay" ) );
            topLayout->addLayout( layout );
        }
    }
    
    set( acqrscontrols::ap240::method() );
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
    onInitialUpdate();
}

void
ap240widget::onInitialUpdate()
{
    if ( auto w = findChild< AcqirisWidget * >() ) {
        w->onInitialUpdate();
        w->setContents( acqrscontrols::ap240::method() );

        w->setStyleSheet( "QTreeView { background: #e8f4fc; }\n"
                          "QTreeView::item:open { background-color: #1d3dec; color: white; }" );

        connect( w, &AcqirisWidget::dataChanged, [&]( const AcqirisWidget *, int cat ){
                emit valueChanged( idAP240Any, 0 );
                emit dataChanged();
            });
        connect( w, &AcqirisWidget::stateChanged, [&]( const QModelIndex& index, bool ){
                emit valueChanged( idChannels, index.row() - 2 );
                emit dataChanged();
            });            
    }
    if ( auto edit = findChild< QLineEdit * >( "external_delay" ) )
        edit->setReadOnly( true );
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
ap240widget::onStatus( int )
{
}

void
ap240widget::get( std::shared_ptr< acqrscontrols::ap240::method > m ) const
{
    get( *m );
}

void
ap240widget::set( std::shared_ptr< const acqrscontrols::ap240::method> m )
{
    set( *m );
}

bool
ap240widget::get( acqrscontrols::ap240::method& m ) const
{
    if ( auto w = findChild< AcqirisWidget * >() ) {
        w->getContents( m );
#ifndef NDEBUG
        // std::ostringstream o;
        // o << " Ext. trig. delay: ";
        // for ( int i = 0; i < m.protocols().size(); ++i )
        //     o << boost::format("[%1%] %2%; ") % i % m.protocols() [ i ].delay_pulses() [ adcontrols::TofProtocol::EXT_ADC_TRIG ].first;
        // ADDEBUG() << __FUNCTION__ << o.str();
#endif        
        return true;
    }
    return false;
}

bool
ap240widget::set( const acqrscontrols::ap240::method& m )
{
    if ( auto w = findChild< AcqirisWidget * >() ) {
        w->setContents( m );
    }

    if ( auto edit = findChild< QLineEdit * >( "external_delay" ) ) {
        QString text;
        for ( int i = 0; i < m.protocols().size(); ++i ) {
            auto d = ( boost::format("[%d]: %.2fus; ")
                       % i
                       % ( m.protocols()[i].delay_pulses()[ adcontrols::TofProtocol::EXT_ADC_TRIG ].first * std::micro::den ) ).str();
            text += QString::fromStdString( d );
        }
        edit->setText( text );
    }

    return true;
}
