/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "devicefacade.hpp"
#include "toftask.hpp"
#include "data_simulator.hpp"

using namespace tofservant;

namespace tofservant {
    struct method_visitor : public boost::static_visitor<void> {
        const TOF::ControlMethod& m_;
        method_visitor( const TOF::ControlMethod& m ) : m_( m ) {}
        template< typename T > void operator ()( T& t ) const { 
            t->peripheral_async_apply_method( m_ );
        }
    };

    struct initialize_visitor : public boost::static_visitor<void> {
        template< typename T > void operator ()( T& t ) const { 
            t->peripheral_initialize();
        }
    };

    struct terminate_visitor : public boost::static_visitor<void> {
        template< typename T > void operator ()( T& t ) const { 
            t->peripheral_terminate();
        }
    };

}

DeviceFacade::~DeviceFacade() 
{
}

DeviceFacade::DeviceFacade() : initialized_( false )
{
	vec_.push_back( std::shared_ptr< avgr_emu >( new avgr_emu ) );
}

bool
DeviceFacade::setConfiguration( const char * xml )
{
	(void)xml;
	return true;
}

bool
DeviceFacade::setControlMethod( const TOF::ControlMethod& m, const char * hint)
{
	(void)hint;
    for ( auto& device: vec_ ) 
        boost::apply_visitor( method_visitor( m ), device );
	return true;
}

bool
DeviceFacade::initialize()
{
    for ( auto& device: vec_ ) 
        boost::apply_visitor( initialize_visitor(), device );    
	return true;
}

bool
DeviceFacade::terminate()
{
    for ( auto& device: vec_ ) 
        boost::apply_visitor( terminate_visitor(), device );    
	return true;
}
