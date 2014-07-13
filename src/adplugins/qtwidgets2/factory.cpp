// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "factory.hpp"
#include <QtCore/qplugin.h>
#include "mscalibrationform.hpp"
#include "mspeakview.hpp"
#include "mschromatogramwidget.hpp"
#include <adplugin/lifecycle.hpp>
#include <adplugin/adplugin.hpp>
#include <adplugin/constants.hpp>
#include <adlog/logger.hpp>
#include <qdebug.h>
#include <QMessageBox>

using namespace qtwidgets2;

QWidget *
factory::create_widget( const wchar_t * iid, QWidget * parent )
{
    QWidget * pWidget = 0;
    if ( std::wstring( iid ) == L"qtwidgets2::MSCalibrationForm" ) {
        pWidget = new qtwidgets2::MSCalibrationForm( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets2::MSPeakView" ) {
        pWidget = new qtwidgets2::MSPeakView( parent );
    } else if ( std::wstring( iid ) == L"qtwidgets2::MSChromatogramWidget" ) {
        pWidget = new qtwidgets2::MSChromatogramWidget( parent );
    }
    if ( pWidget )
        return pWidget;
    return 0;
}

QObject *
factory::create_object( const wchar_t * iid, QObject * parent )
{
    Q_UNUSED( parent );
    Q_UNUSED( iid );
    return 0;
}

void
factory::release()
{
}

factory * factory::instance_ = 0;

factory *
factory::instance()
{
	if ( instance_ == 0 )
		instance_ = new factory();
	return instance_;
}

void *
factory::query_interface_workaround( const char * _typenam )
{
    const std::string typenam( _typenam );

    if ( typenam == typeid( adplugin::widget_factory ).name() )
        return static_cast< adplugin::widget_factory * >( this );

    return 0;
}

void
factory::accept( adplugin::visitor& v, const char * adpluginspec )
{
	v.visit( this, adpluginspec );
}

const char *
factory::iid() const
{
	return "com.ms-cheminfo.qtplatz.adplugins.widget_factory.qtplatz2";
}

EXPORT_FACTORY( qtwidgets2::factory )
