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

#include "accutof.hpp"
#include <adcontrols/visitor.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/visitor.hpp>
#include "accutofinterpreter.hpp"
#include "accutofscanlaw.hpp"
#include "constants.hpp"

using namespace accutofspectrometer;

accutof * accutof::instance_ = 0;

accutof *
accutof::instance()
{
    if ( instance_ == 0 ) {
        instance_ = new accutof;
        atexit( accutof::dispose );
    }
    return instance_;
}

void
accutof::dispose()
{
    if ( instance_ ) {
        delete instance_;
        instance_ = 0;
    }
}

const char *
accutof::iid() const
{
    return "com.ms-cheminfo.qtplatz.adplugins.massSpectrometer.accutof";
}

void
accutof::accept( adplugin::visitor& v, const char * adpluginspec )
{
	v.visit( this, adpluginspec );
}

void *
accutof::query_interface_workaround( const char * typenam )
{
	std::string tname( typenam );

	if ( tname == typeid( adcontrols::MassSpectrometer ).name() )
		return static_cast< adcontrols::MassSpectrometer * >( this );

	if ( tname == typeid( adcontrols::massspectrometer_factory ).name() )
		return static_cast< adcontrols::massspectrometer_factory *>( this );

    return 0;
}

accutof::~accutof()
{
}

accutof::accutof() : scanLaw_( new accutofScanLaw )
           , interpreter_( new accutofInterpreter )
{
}

void
accutof::accept( adcontrols::Visitor& visitor )
{
    visitor.visit( *this );
}

const wchar_t *
accutof::name() const
{
    return constants::dataInterpreter::spectrometer::name();
}

adcontrols::MassSpectrometer *
accutof::get( const wchar_t * name )
{
	(void)name;
	return this;
}

const adcontrols::MassSpectrometer::ScanLaw&
accutof::getScanLaw() const
{
    return *scanLaw_;
}

const adcontrols::DataInterpreter&
accutof::getDataInterpreter() const
{
    return *interpreter_;
}
