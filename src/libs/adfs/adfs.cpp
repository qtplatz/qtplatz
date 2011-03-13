// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "adfs.h"
#include <boost/smart_ptr.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include "superblock.h"
#include <fstream>

using namespace adfs;

namespace adfs {
    namespace internal {

        class disk_image {
            //boost::interprocess::file_mapping *mmap;
            boost::interprocess::managed_mapped_file *mmap;
            disk_super_block * sb;
        public:
            ~disk_image();
            disk_image();
            disk_image( const char * filename, adfs::mode_t mode );
        };
    }
}

disk::~disk()
{
    delete image;
}

disk::disk() : image( 0 )
{
}

disk::disk( const disk& t ) : image( new internal::disk_image( *t.image ) )
{
}

disk
disk::mount( const char * /* filename */, bool /* readonly */ )
{
    disk disk;
    return disk;
}

disk
disk::create( const char * filename )
{
    disk disk;
    disk.image = new internal::disk_image( filename, adfs::disk_create );
    return disk;
}

////////////////

using namespace adfs::internal;

disk_image::~disk_image()
{
    delete sb;
    delete mmap;
}

disk_image::disk_image() : mmap(0), sb(0)
{
}

disk_image::disk_image( const char * filename, adfs::mode_t cmode )
{
    std::size_t pagesize = boost::interprocess::mapped_region::get_page_size();
    
    if ( cmode == adfs::disk_create ) {
        // boost::interprocess::managed_mapped_file::remove( filename );
        //mmap = new boost::interprocess::managed_mapped_file( boost::interprocess::open_or_create, filename, pagesize );
        boost::interprocess::managed_mapped_file::grow( filename, pagesize );
        boost::interprocess::managed_mapped_file map( boost::interprocess::open_or_create, filename, pagesize );

/*
        std::filebuf fbuf;
        fbuf.open( filename, std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
        mmap = new boost::interprocess::file_mapping( filename, boost::interprocess::read_write );
        boost::interprocess::mapped_region region( *mmap, boost::interprocess::read_write, pagesize );
        sb = new ( region.get_address() ) disk_super_block;
*/
    } else if ( cmode == adfs::disk_readonly ) {
/*
        mmap = new boost::interprocess::file_mapping( filename, boost::interprocess::read_only );
        boost::interprocess::mapped_region region( *mmap, boost::interprocess::read_write, pagesize );
        sb = reinterpret_cast< disk_super_block * >( region.get_address() );     
*/
    } else {
/*
        mmap = new boost::interprocess::file_mapping( filename, boost::interprocess::read_write );
        boost::interprocess::mapped_region region( *mmap, boost::interprocess::read_only, pagesize );
        sb = reinterpret_cast< disk_super_block * >( region.get_address() );
*/
    }
}