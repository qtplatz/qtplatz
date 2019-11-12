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

#include "aqmd3widget.hpp"
#include "aqmd3form.hpp"
#include "aqmd3table.hpp"
#include <aqmd3/digitizer.hpp>
#include <aqmd3controls/method.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adinterface/controlserver.hpp>
#include <adportable/is_type.hpp>
#include <adportable/serializer.hpp>


#if defined _MSC_VER
# pragma warning( disable: 4251 )
#endif

#include <QSplitter>
#include <QBoxLayout>
#include <QMessageBox>
#include <boost/exception/all.hpp>

using namespace aqmd3widgets;

AQMD3Widget::AQMD3Widget(QWidget *parent) : QWidget(parent)
{
    if ( QSplitter * splitter = new QSplitter ) {

        splitter->addWidget( new aqmd3Form(this) );
        splitter->addWidget( new aqmd3Table(this) );
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
AQMD3Widget::onInitialUpdate()
{
    if ( auto form = findChild< aqmd3Form * >() ) {
        if ( auto table = findChild< aqmd3Table * >() ) {

            form->onInitialUpdate();
            form->setContents( aqmd3controls::method() );

            connect( form, &aqmd3Form::valueChanged, [this, table] ( idCategory id, int channel, QVariant value ) {
                    table->onHandleValue( id, channel, value );
                    if ( id == idAQMD3Any || id == idTSREnable ) {
                        emit applyTriggered();      // apply method to hardware
                        emit valueChanged( id, channel ); // for previous compatibility, will be deprecated
                    } else {
                        emit dataChanged(); // give a chance to update UI-Complex (such as InfiTOF/AQMD3 combination)
                    }
                } );


            table->onInitialUpdate();
            table->setContents( aqmd3controls::device_method() );

            connect( table, &aqmd3Table::valueChanged, [this, form] ( idCategory id, int channel, QVariant value ) {
                    form->onHandleValue( id, channel, value );
                    emit dataChanged(); // give a chance to update UI-Complex (such as InfiTOF/AQMD3 combination)
                } );
        }
    }
}

void
AQMD3Widget::onStatus( int st )
{
}

void
AQMD3Widget::OnCreate( const adportable::Configuration& )
{
}

void
AQMD3Widget::OnInitialUpdate()
{
    onInitialUpdate();
}

void
AQMD3Widget::OnFinalClose()
{
}

bool
AQMD3Widget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        aqmd3controls::method m;

        adcontrols::ControlMethodPtr ptr = boost::any_cast<adcontrols::ControlMethodPtr>( a );
        auto it = ptr->find( ptr->begin(), ptr->end(), aqmd3controls::method::clsid() );

        if ( it != ptr->end() )
            it->get( *it, m ); // save threshold method

        get( m ); // override

        ptr->append( m );

        return true;

    }
    return false;
}

bool
AQMD3Widget::setContents( boost::any&& a )
{
    auto pi = adcontrols::ControlMethod::any_cast<>()( a , aqmd3controls::method::clsid());
	if (pi) {
        aqmd3controls::method m;
		try {

            if ( pi->get<>( *pi, m ) )
                return set( m );

		} catch (boost::exception& ex) {
			QMessageBox::warning(this, "AQMD3 Method", QString::fromStdString(boost::diagnostic_information(ex)));
		} catch ( ... ) {
			QMessageBox::warning(this, "AQMD3 Method", QString::fromStdString(boost::current_exception_diagnostic_information()));
		}
    }
    return false;
}

bool
AQMD3Widget::get( aqmd3controls::method& m ) const
{
    if ( auto form = findChild< aqmd3Form * >() ) {
        form->getContents( m );
    }
    if ( auto table = findChild< aqmd3Table *>() ) {
        table->getContents( m.device_method() );
    }
    return true;
}

bool
AQMD3Widget::set( const aqmd3controls::method& m )
{
    if ( auto form = findChild< aqmd3Form * >() ) {
        form->setContents( m );
    }
    if ( auto table = findChild< aqmd3Table *>() ) {
        table->setContents( m.device_method() );
    }
    return true;
}

void
AQMD3Widget::setEnabled( const QString& name, bool enable )
{
    if ( auto form = findChild< aqmd3Form * >() ) {
        form->setEnabled( name, enable );
    }
    if ( auto table = findChild< aqmd3Table *>() ) {
        table->setEnabled( name, enable );
    }
}
