/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "adpluginfactory.hpp"
#include <adplugin/adplugin.hpp>
#include <adplugin/visitor.hpp>
#include <adplugin/constants.hpp>
#include <QtCore/qplugin.h>
#include "monitorui.hpp"

using namespace toftune;

adpluginfactory * adpluginfactory::instance_ = 0;

adpluginfactory::adpluginfactory()
{
}

adpluginfactory *
adpluginfactory::instance()
{
    if ( instance_ == 0 )
        instance_ = new adpluginfactory;
    return instance_;
}

QWidget *
adpluginfactory::create_widget( const wchar_t * iid, QWidget * parent )
{
	if ( std::wstring( iid ) == iid_iMonitor )
		return new MonitorUI( parent );
	return 0;
}

QObject *
adpluginfactory::create_object( const wchar_t *, QObject * parent )
{
    Q_UNUSED( parent );
	return 0;
}

void
adpluginfactory::release()
{
    delete this;
}

//
void *
adpluginfactory::query_interface_workaround( const char * _typenam )
{
    const std::string typenam( _typenam );

    if ( typenam == typeid( adplugin::widget_factory ).name() )
        return static_cast< adplugin::widget_factory * >( this );

    return 0;
}

void
adpluginfactory::accept( adplugin::visitor& v, const char * adpluginspec )
{
    v.visit( this, adpluginspec );
}

const char *
adpluginfactory::iid() const
{
    return "com.ms-cheminfo.qtplatz.adplugins.widget_factory.tof";
}

EXPORT_FACTORY( adpluginfactory )
