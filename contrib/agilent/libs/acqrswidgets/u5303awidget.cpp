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

#include "u5303awidget.hpp"
#include "u5303aform.hpp"
#include "u5303atable.hpp"
#include <u5303a/digitizer.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adinterface/controlserver.hpp>
#include <adportable/is_type.hpp>
#include <adportable/serializer.hpp>
#include <u5303a/digitizer.hpp>
#include <acqrscontrols/u5303a/method.hpp>

#if defined _MSC_VER
# pragma warning( disable: 4251 )
#endif

#include <QSplitter>
#include <QBoxLayout>
#include <QMessagebox>
#include <boost/exception/all.hpp>

using namespace acqrswidgets;

u5303AWidget::u5303AWidget(QWidget *parent) : QWidget(parent)
{
    if ( QSplitter * splitter = new QSplitter ) {

        splitter->addWidget( new u5303AForm(this) );
        splitter->addWidget( new u5303ATable(this) );
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
u5303AWidget::onInitialUpdate()
{
    if ( auto form = findChild< u5303AForm * >() ) {
        if ( auto table = findChild< u5303ATable * >() ) {

            form->onInitialUpdate();
            form->setContents( acqrscontrols::u5303a::method() );

            connect( form, &u5303AForm::valueChanged, [this, table] ( idCategory id, int channel, QVariant value ) {
                    table->onHandleValue( id, channel, value );
                    emit valueChanged( id, channel );
                } );
            
            
            table->onInitialUpdate();
            table->setContents( acqrscontrols::u5303a::device_method() );

            connect( table, &u5303ATable::valueChanged, [this, form] ( idCategory id, int channel, QVariant value ) {
                    form->onHandleValue( id, channel, value );
                    emit valueChanged( id, channel );
                } );
        }
    }
}

void
u5303AWidget::onStatus( int st )
{
}

void
u5303AWidget::OnCreate( const adportable::Configuration& )
{
}

void
u5303AWidget::OnInitialUpdate()
{
    onInitialUpdate();
}

void
u5303AWidget::OnFinalClose()
{
}

bool
u5303AWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        acqrscontrols::u5303a::method m;
        get( m );

        adcontrols::ControlMethodPtr ptr = boost::any_cast<adcontrols::ControlMethodPtr>( a );
        ptr->append( m );

        return true;

    }
    return false;
}

bool
u5303AWidget::setContents( boost::any& a )
{
    const adcontrols::ControlMethod::MethodItem * pi(0);
    if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_pointer( a ) ) {
        pi = boost::any_cast<const adcontrols::ControlMethod::MethodItem * >( a );             
    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_a( a ) ) {   
        pi = &boost::any_cast<const adcontrols::ControlMethod::MethodItem& >( a );
    }

    if ( !pi ) {
        std::shared_ptr< const adcontrols::ControlMethod::Method > ptr;
        if ( adportable::a_type< std::shared_ptr< const adcontrols::ControlMethod::Method > >::is_a( a ) ) {
            ptr = boost::any_cast<std::shared_ptr< const adcontrols::ControlMethod::Method >>( a );
        } else if ( adportable::a_type< std::shared_ptr< adcontrols::ControlMethod::Method > >::is_a( a ) ) {
            ptr = boost::any_cast<std::shared_ptr< adcontrols::ControlMethod::Method >>( a );
        }
        if ( ptr ) {
            auto it = ptr->find( ptr->begin(), ptr->end(), acqrscontrols::u5303a::method::modelClass() );
            if ( it != ptr->end() )
                pi = &( *it );
        }
    }

	if (pi) {
        acqrscontrols::u5303a::method m;
		try {

            if ( pi->get<>( *pi, m ) ) 
                return set( m );

		} catch (boost::exception& ex) {
			QMessageBox::warning(this, "U5303A Method", QString::fromStdString(boost::diagnostic_information(ex)));
		} catch ( ... ) {
			QMessageBox::warning(this, "U5303A Method", QString::fromStdString(boost::current_exception_diagnostic_information()));
		}
    }
    return false;
}

bool
u5303AWidget::get( acqrscontrols::u5303a::method& m ) const
{
    if ( auto form = findChild< u5303AForm * >() ) {
        form->getContents( m );
    }
    if ( auto table = findChild< u5303ATable *>() ) {
        table->getContents( m.method_ );
    }
    return true;
}

bool
u5303AWidget::set( const acqrscontrols::u5303a::method& m )
{
    if ( auto form = findChild< u5303AForm * >() ) {
        form->setContents( m );
    }
    if ( auto table = findChild< u5303ATable *>() ) {
        table->setContents( m.method_ );
    }
    return true;
}
