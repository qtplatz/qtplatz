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

#include "document.hpp"
#include <u5303a/digitizer.hpp>
#include <adlog/logger.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adinterface/controlserver.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <string>

using namespace u5303a;

document * document::instance_ = 0;
std::mutex document::mutex_;

namespace u5303a { namespace detail {
        struct remover {
            ~remover() {
                if ( document::instance_ ) {
                    std::lock_guard< std::mutex > lock( document::mutex_ );
                    if ( document::instance_ )
                        delete document::instance_;
                }
            };
            static remover _remover;
        };
    }
}
    
document::document() : digitizer_( new u5303a::digitizer )
                     , method_( std::make_shared< u5303a::method >() )
                     , device_status_( 0 )
{
}

document::~document()
{
    delete digitizer_;
}

document *
document::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new document;
    }
    return instance_;
}

void
document::u5303a_connect()
{
    digitizer_->connect_reply( boost::bind( &document::reply_handler, this, _1, _2 ) );
    digitizer_->connect_waveform( boost::bind( &document::waveform_handler, this, _1 ) );
    digitizer_->peripheral_initialize();
}

void
document::prepare_for_run()
{
	digitizer_->peripheral_prepare_for_run( *method_ );
}

void
document::prepare_for_run( const u5303a::method& m )
{
    *method_ = m;
	digitizer_->peripheral_prepare_for_run( *method_ );
}

void
document::u5303a_start_run()
{
	digitizer_->peripheral_run();
}

void
document::u5303a_stop()
{
	digitizer_->peripheral_stop();
}

void
document::u5303a_trigger_inject()
{
	digitizer_->peripheral_trigger_inject();
}

int32_t
document::device_status() const
{
    return device_status_;
}

void
document::reply_handler( const std::string& method, const std::string& reply )
{
	emit on_reply( QString::fromStdString( method ), QString::fromStdString( reply ) );
    if ( method == "InitialSetup" && reply == "success" ) {
        device_status_ = controlserver::eStandBy;
        emit on_status( device_status_ );
    }
}

void
document::waveform_handler( const waveform * p )
{
    auto ptr = p->shared_from_this();
    std::lock_guard< std::mutex > lock( mutex_ );
    while ( que_.size() >= 32 )
        que_.pop_front();
	que_.push_back( ptr );
    emit on_waveform_received();
}

std::shared_ptr< const waveform >
document::findWaveform( uint32_t serialnumber )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( que_.empty() )
        return 0;
	std::shared_ptr< const waveform > ptr = que_.back();
	ADTRACE() << "findWaveform: " << ptr->serialnumber_;
    //if ( serialnumber == (-1) )
    return ptr;
	/*
	auto it = std::find_if( que_.begin(), que_.end(), [=]( std::shared_ptr< const waveform >& p ){ return p->serialnumber_ == serialnumber; });
    if ( it != que_.end() )
        return *it;
    */
	return 0;
}

const u5303a::method&
document::method() const
{
    return *method_;
}

// static
bool
document::toMassSpectrum( adcontrols::MassSpectrum& sp, const waveform& waveform )
{
    using namespace adcontrols::metric;

    sp.setCentroid( adcontrols::CentroidNone );

    adcontrols::MSProperty prop = sp.getMSProperty();
    adcontrols::MSProperty::SamplingInfo info( 0
                                               , waveform.method_.delay_to_first_s
                                               , uint32_t(waveform.d_.size())
                                               , waveform.method_.nbr_of_averages + 1
                                               , 0 );
    info.fSampInterval( 1.0 / waveform.method_.samp_rate );
    prop.acceleratorVoltage( 3000 );
    prop.setSamplingInfo( info );
    
    prop.setTimeSinceInjection( scale_to_base<double>( double( waveform.timestamp_ ), pico ) );
    prop.setDataInterpreterClsid( "u5303a" );

    // prop.setDeviceData(); TBA
    sp.setMSProperty( prop );
    sp.resize( waveform.d_.size() );
	int idx = 0;
    for ( auto y: waveform.d_ )
        sp.setIntensity( idx++, y );
    // mass array tba
	return true;
}

// static
bool
document::appendOnFile( const std::wstring& path
                        , const std::wstring& title
                        , const adcontrols::MassSpectrum& ms, std::wstring& id )
{
    adfs::filesystem fs;
	
	if ( ! boost::filesystem::exists( path ) ) {
		if ( ! fs.create( path.c_str() ) )
			return false;
	} else {
		if ( ! fs.mount( path.c_str() ) )
			return false;
	}
	adfs::folder folder = fs.addFolder( L"/Processed/Spectra" );

    if ( folder ) {
		adfs::file file = folder.addFile( adfs::create_uuid(), title );
        if ( file ) {
            file.dataClass( ms.dataClass() );
            id = file.id();
            if ( file.save( ms ) ) //adfs::cpio< adcontrols::MassSpectrum >::save( ms, file ) )
				file.commit();
        }
	}
    return true;
    
}
