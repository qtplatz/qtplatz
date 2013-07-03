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

#include "rawxfer.hpp"
#include <adportable/debug.hpp>
#include <boost/array.hpp> // <tr1/array>
#include "cstdint.hpp"

using namespace tofinterface;

namespace tofinterface {
    namespace constants {
        const char * share_name      = "tofSharedMemory";
        const char * mutex_name      = "tofMutex";
    }
}

const char * 
rawxfer::share_name() 
{
    return constants::share_name;
}

const char * 
rawxfer::mutex_name() 
{
    return constants::mutex_name;
}

rawxfer::~rawxfer()
{
}

rawxfer::rawxfer() : shm_( 0 )
                   , mutex_( 0 )
{
}

bool
rawxfer::sbrk( std::size_t octets )
{
    using namespace boost::interprocess;

    try {
	shm_ = new managed_shared_memory( open_or_create, constants::share_name, octets );
    } catch ( std::exception& ex ) {
        adportable::debug( __FILE__, __LINE__ ) <<
            "tofinterface::rawxfer::sbrk exception: create shared memory: " << ex.what();
        return false;
    }

    try {
        mutex_ = shm_->find_or_construct< interprocess_mutex >( constants::mutex_name )();
    } catch ( std::exception& ex ) {
        shared_memory_object::remove( constants::share_name );
        adportable::debug( __FILE__, __LINE__ ) <<
            "tofinterface::rawxfer::sbrk exception: create mutex: " << ex.what();
        return false;
    }

    return true;
}

#if 0
bool
rawxfer::internal_create()
{
    try {
	// outgoing data memory
	dma * data = shm_->find_or_construct< dma_emulation::dma >( object_name_out )();
	data->numwords_ = 0;
	for ( std::size_t i = 0; i < data->data_.size(); ++i )
	    data->data_[ i ] = i;
    } catch ( std::exception& ex ) {
	std::cout << "tofinterface::rawxfer exception: construct dma: " << ex.what() << std::endl;
	return false;
    }

    try {
	// incoming data memory
	dma * data = shm_->find_or_construct< tofinterface::rawxfer::dma >( object_name_in )();
	data->numwords_ = 0;
	for ( std::size_t i = 0; i < data->data_.size(); ++i )
	    data->data_[ i ] = i;
    } catch ( std::exception& ex ) {
	std::cout << "tofinterface::rawxfer exception: construct dma: " << ex.what() << std::endl;
	return false;
    }

    try {
	mutex_ = shm_->find_or_construct< interprocess_mutex >( mutex_name )();
    } catch ( std::exception& ex ) {
	std::cout << "tofinterface::rawxfer exception: create mutex: " << ex.what() << std::endl;
	return false;
    }
    return true;
}
#endif
