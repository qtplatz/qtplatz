/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include "massspectrometer.hpp"
#include "datainterpreter.hpp"
#include <adcontrols/msproperty.hpp>
#include <adplugin/visitor.hpp>
#include <adportable/timesquaredscanlaw.hpp>

namespace u5303aspectrometer {

    // base class in adcontrols/massspectrometer.hpp

    class ScanLaw : public adcontrols::ScanLaw {
        adportable::TimeSquaredScanLaw law_;
    public:
        ~ScanLaw() {}
        ScanLaw() : law_( 3700, 0.34e-6, 1.0 ) {
        }
        ScanLaw( const ScanLaw& t ) : law_( t.law_ ) {
        }

        // adcontrols::ScanLaw
        double getMass( double t, int mode ) const override {
            return law_.getMass( t, mode );
        }
        double getTime( double m, int mode ) const override {
            return law_.getTime( m, mode );
        }
        double getMass( double t, double fLength ) const override {
            return law_.getMass( t, fLength );
        }
        double getTime( double m, double fLength ) const override {
            return law_.getTime( m, fLength );
        }
        double fLength( int type ) const override {
            return law_.fLength( type );
        }
    };

}

using namespace u5303aspectrometer;

std::atomic< MassSpectrometer * > MassSpectrometer::instance_( 0 );
std::mutex MassSpectrometer::mutex_;

MassSpectrometer::~MassSpectrometer()
{
}

MassSpectrometer::MassSpectrometer() : scanlaw_( std::make_shared< ScanLaw >() )
                                     , interpreter_( std::make_shared< DataInterpreter >() )
{
}

MassSpectrometer::MassSpectrometer( adcontrols::datafile * file ) : adcontrols::MassSpectrometer( file )
                                                                  , interpreter_( std::make_shared< DataInterpreter >() )
                                                                  , scanlaw_( std::make_shared< ScanLaw >() )
{
}

//static
MassSpectrometer *
MassSpectrometer::instance()
{
    typedef MassSpectrometer T;
    T * tmp = instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        tmp = instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new T();
            std::atomic_thread_fence( std::memory_order_release );
            instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
}

//static
void
MassSpectrometer::dispose()
{
}

// adcontrols::MassSpectrometer
const adcontrols::ScanLaw&
MassSpectrometer::getScanLaw() const
{
    return *scanlaw_;
}

std::shared_ptr< adcontrols::ScanLaw >
MassSpectrometer::scanLaw( const adcontrols::MSProperty& ) const
{
    return std::make_shared< ScanLaw >();
}

const adcontrols::DataInterpreter&
MassSpectrometer::getDataInterpreter() const
{
    return *interpreter_;
}

const wchar_t * 
MassSpectrometer::name() const 
{
    return L"u5303a";
}

// massspectrometer_factory
adcontrols::MassSpectrometer *
MassSpectrometer::get( const wchar_t * /* name */)
{
    return this;
}

std::shared_ptr< adcontrols::MassSpectrometer >
MassSpectrometer::create( const wchar_t *, adcontrols::datafile * file ) const
{
    std::shared_ptr< adcontrols::MassSpectrometer > ptr = std::make_shared< MassSpectrometer >( file );
    return ptr;
}

// plugin
const char *
MassSpectrometer::iid() const
{
    return "com.ms-cheminfo.qtplatz.adplugins.massSpectrometer.u5303a";
}

void
MassSpectrometer::accept( adplugin::visitor& v, const char * pluginspecs )
{
    v.visit( this, pluginspecs );
}

void *
MassSpectrometer::query_interface_workaround( const char * typenam )
{
	std::string tname( typenam );
    
	if ( tname == typeid( adcontrols::MassSpectrometer ).name() )
		return static_cast< adcontrols::MassSpectrometer * >( this );

	if ( tname == typeid( adcontrols::massspectrometer_factory ).name() )
		return static_cast< adcontrols::massspectrometer_factory *>( this );

    return 0;
}
