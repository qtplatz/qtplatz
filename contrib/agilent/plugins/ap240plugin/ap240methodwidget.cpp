/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "ap240methodwidget.hpp"
#include "ap240form.hpp"
#include "ap240methodtable.hpp"
#include "document.hpp"
#include <ap240/digitizer.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adinterface/controlserver.hpp>
#include <adportable/is_type.hpp>
#include <adportable/serializer.hpp>
#include <ap240/digitizer.hpp>
#include <QSplitter>
#include <QBoxLayout>
#include <QMessageBox>
#include <boost/exception/all.hpp>

using namespace ap240;

ap240MethodWidget::ap240MethodWidget(QWidget *parent) : QWidget(parent)
{
    if ( QSplitter * splitter = new QSplitter ) {

        splitter->addWidget( new ap240Form(this) );
        splitter->addWidget( new ap240MethodTable(this) );
        splitter->setOrientation( Qt::Horizontal );
        splitter->setStretchFactor( 0, 1 );
        splitter->setStretchFactor( 1, 4 );

        if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {
            layout->setMargin( 0 );
            layout->setSpacing( 0 );
            layout->addWidget( splitter );
        }
    }
}

void
ap240MethodWidget::onInitialUpdate()
{
    if ( auto form = findChild< ap240Form * >() ) {
        form->onInitialUpdate();
        connect( form, &ap240Form::trigger_apply, this, &ap240MethodWidget::handle_trigger_apply );
    }

    if ( auto table = findChild< ap240MethodTable * >() ) {
        table->onInitialUpdate();
        table->setContents( ap240::method() );
    }
}

void
ap240MethodWidget::onStatus( int st )
{
    if ( auto form = findChild< ap240Form * >() )
        form->onStatus( st );        
}

void
ap240MethodWidget::handle_trigger_apply()
{
    document::instance()->prepare_for_run();
    /*
    if ( auto table = findChild< ap240MethodTable * >() ) {
        ap240::method m;
        if ( table->getContents( m ) ) {
            document::instance()->prepare_for_run( m );
        }
    }
    */
    
}

void
ap240MethodWidget::OnCreate( const adportable::Configuration& )
{
}

void
ap240MethodWidget::OnInitialUpdate()
{
    onInitialUpdate();
}

void
ap240MethodWidget::OnFinalClose()
{
}

void
ap240MethodWidget::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = this;
}

bool
ap240MethodWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        adcontrols::ControlMethodPtr ptr = boost::any_cast<adcontrols::ControlMethodPtr>(a);        

        ap240::method m;
        
        std::string device;
        adportable::serializer< ap240::method >::serialize( m, device );

        adcontrols::controlmethod::MethodItem item;
        item.setModelname( "ap240" );
        item.isInitialCondition( true );
        item.setItemLabel( "ap240" );
        item.unitnumber( 1 );
        item.funcid( 1 );
        item.data( device.data(), device.size() );
        ptr->insert( item );
        return true;        
    }
    else if ( adportable::a_type< adcontrols::controlmethod::MethodItem >::is_pointer( a ) ) {

        auto pi = boost::any_cast<adcontrols::controlmethod::MethodItem * >( a );                

        ap240::method m;
        if ( auto table = findChild< ap240MethodTable * >() ) {
            table->getContents( m );

            std::string device;
            adportable::serializer< ap240::method >::serialize( m, device );
            
            pi->setModelname( "ap240" );
            pi->setItemLabel( "ap240" );
            pi->unitnumber( 1 );
            pi->funcid( 1 );
            pi->data( device.data(), device.size() );
            return true;
        }
    }
    return false;
}

bool
ap240MethodWidget::setContents( boost::any& a )
{
    const adcontrols::controlmethod::MethodItem * pi(0);
    if ( adportable::a_type< adcontrols::controlmethod::MethodItem >::is_pointer( a ) ) {
        pi = boost::any_cast<const adcontrols::controlmethod::MethodItem * >( a );             
    } else if ( adportable::a_type< adcontrols::controlmethod::MethodItem >::is_a( a ) ) {   
        pi = &boost::any_cast<const adcontrols::controlmethod::MethodItem& >( a );
    }
	if (pi) {
		ap240::method m;
		try {
			if (adportable::serializer< ap240::method >::deserialize(m, pi->data(), pi->size())) {
				if (auto table = findChild< ap240MethodTable * >()) {
					table->setContents(m);
					return true;
				}
			}
		} catch (boost::exception& ex) {
			QMessageBox::warning(this, "AP240 Method", QString::fromStdString(boost::diagnostic_information(ex)));
		} catch ( ... ) {
			QMessageBox::warning(this, "AP240 Method", QString::fromStdString(boost::current_exception_diagnostic_information()));
		}
    }
    return false;
}
