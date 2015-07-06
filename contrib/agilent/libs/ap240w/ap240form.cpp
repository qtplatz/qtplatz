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

#include "ap240form.hpp"
#include "ap240horizontalform.hpp"
#include "ap240verticalform.hpp"
#include "ap240triggerform.hpp"
#include "ui_ap240form.h"
#include <ap240/digitizer.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adportable/is_type.hpp>
#include <adportable/serializer.hpp>
#include <QVBoxLayout>
#include <QMessageBox>
#include <boost/exception/all.hpp>

ap240form::ap240form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ap240form)
{
    ui->setupUi(this);
    if ( auto layout = new QVBoxLayout( ui->groupBox ) ) {
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( new ap240TriggerForm );
    }
    if ( auto layout = new QVBoxLayout( ui->groupBox_4 ) ) {
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( new ap240HorizontalForm );
    }
    for ( auto& w: { ui->groupBox_2, ui->groupBox_3, ui->groupBox_5 } ) {
        auto layout = new QVBoxLayout( w );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( new ap240VerticalForm );
    }
}

ap240form::~ap240form()
{
    delete ui;
}

// LifeCycle
void
ap240form::OnCreate( const adportable::Configuration& )
{
}

void
ap240form::OnInitialUpdate()
{
}

void
ap240form::OnFinalClose()
{
}

bool
ap240form::getContents( boost::any& a ) const
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
        
    } else if ( adportable::a_type< adcontrols::controlmethod::MethodItem >::is_pointer( a ) ) {
        
        auto pi = boost::any_cast<adcontrols::controlmethod::MethodItem * >( a );                
#if 0
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
#endif
    }
    return false;
}

bool
ap240form::setContents( boost::any& a )
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
            adportable::serializer< ap240::method >::deserialize( m, pi->data(), pi->size() );

		} catch (boost::exception& ex) {
			QMessageBox::warning(this, "AP240 Method", QString::fromStdString(boost::diagnostic_information(ex)));
		} catch ( ... ) {
			QMessageBox::warning(this, "AP240 Method", QString::fromStdString(boost::current_exception_diagnostic_information()));
		}

        if ( auto form = findChild< ap240HorizontalForm *>() ) {
        }
        if ( auto form = findChild< ap240TriggerForm *>() ) {
        }
        auto forms = findChildren< ap240VerticalForm * >();
        for ( auto form : forms ) {
        }
    }
    return true;
}

void
ap240form::onInitialUpdate()
{
}

void
ap240form::onStatus( int )
{
}

void
ap240form::load( std::shared_ptr< adcontrols::ControlMethod > ptr )
{
}
