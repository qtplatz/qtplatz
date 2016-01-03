/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "massspectrometerfactory.hpp"
#include "datainterpreter.hpp"
#include "massspectrometer.hpp"
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adplugin/visitor.hpp>
#include <mutex>

using namespace adspectrometer;

// adplugin is managed by ref_count
std::shared_ptr< MassSpectrometerFactory > MassSpectrometerFactory::instance_ = 0;

static std::once_flag flag;

MassSpectrometerFactory *
MassSpectrometerFactory::instance()
{
    //std::call_once( flag, [] () { instance_ = std::make_shared< MassSpectrometerFactory >(); } );
    return 0; // instance_.get();
}


using namespace adspectrometer;

#if 0

MassSpectrometerFactory::MassSpectrometerFactory() //: spectrometer_( new MassSpectrometer(0) )
{
}

MassSpectrometerFactory::~MassSpectrometerFactory()
{
}

const wchar_t * 
MassSpectrometerFactory::name() const
{
    return L"adspectrometer::import";
}

bool
MassSpectrometerFactory::is_canonical_name( const wchar_t * name ) const
{
    for ( auto& alias: { L"batchproc::import", L"adspectrometer::import" } ) {
        if ( std::wcscmp( alias, name ) == 0 )
            return true;
    }
    return false;
}

adcontrols::MassSpectrometer *
MassSpectrometerFactory::get( const wchar_t * modelname )
{
    if ( std::wcscmp( L"batchproc::import", modelname ) == 0 )
        return spectrometer_.get();

    if ( std::wcscmp( L"adspectrometer::import", modelname ) == 0 )
        return spectrometer_.get();

    return 0;
}

std::shared_ptr< adcontrols::MassSpectrometer >
MassSpectrometerFactory::create( const wchar_t * modelname, adcontrols::datafile * datafile ) const
{
    if ( std::wcscmp( L"batchproc::import", modelname ) == 0 )
        return std::make_shared< adspectrometer::MassSpectrometer >( datafile );

    if ( std::wcscmp( L"adspectrometer::import", modelname ) == 0 )
        return std::make_shared< adspectrometer::MassSpectrometer >( datafile );

    return 0;
}

///////  plugin impl
const char *
MassSpectrometerFactory::iid() const 
{
    return "com.ms-cheminfo.qtplatz.adplugins.massSpectrometer.adspectrometer";
}

void
MassSpectrometerFactory::accept( adplugin::visitor& v, const char * adplugin )
{
	v.visit( this, adplugin );
}

void *
MassSpectrometerFactory::query_interface_workaround( const char * typenam )
{
    if ( std::string( typenam ) == typeid( adcontrols::massspectrometer_factory ).name() )
        return static_cast<adcontrols::massspectrometer_factory *>( this );
    return 0;
}
#endif