// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "datareader.hpp"
#include <map>

namespace adcontrols {

    class DataReader::impl {
    public:
        static impl * instance() {
            static impl __impl__;
            return &__impl__;
        }
        ~impl() {}
        impl() {}

        std::shared_ptr< DataReader > make_reader( const char * traceid ) const;

        std::map< std::string, std::function< factory_type > > reader_factories_;
        std::map< std::string, std::string > reader_map_;  // <traceid, clsid>
    };

}

using namespace adcontrols;

DataReader_iterator::DataReader_iterator( const DataReader& reader, int64_t rowid ) : reader_( reader ), rowid_( rowid )
{
}

const DataReader_iterator&
DataReader_iterator::operator ++ ()
{
    rowid_ = reader_.next( rowid_ );
    return *this;
}

const DataReader_iterator
DataReader_iterator::operator ++ ( int )
{
    DataReader_iterator temp( *this );
    reader_.next( rowid_ );
    return temp;
}

int64_t
DataReader_iterator::pos() const
{
    return reader_.pos( rowid_ );
}

int64_t
DataReader_iterator::elapsed_time() const
{
    return reader_.elapsed_time( rowid_ );
}

double
DataReader_iterator::time_since_inject() const
{
    return reader_.time_since_inject( rowid_ );
}

int
DataReader_iterator::fcn() const
{
    return reader_.fcn( rowid_ );
}


/////////////////////

DataReader::DataReader( const char * traceid )
{
}

DataReader::~DataReader()
{
}

//static
std::shared_ptr< DataReader >
DataReader::make_reader( const char * traceid )
{
    return impl::instance()->make_reader( traceid );
}

//static
void
DataReader::register_factory( std::function< factory_type > f, const char * clsid )
{
    impl::instance()->reader_factories_[ clsid ] = f;
}

//static
void
DataReader::assign_reader( const char * clsid, const char * traceid )
{
    impl::instance()->reader_map_[ traceid ] = clsid;
}

////////////
std::shared_ptr< DataReader >
DataReader::impl::make_reader( const char * traceid ) const
{
    auto it = reader_map_.find( traceid );
    if ( it != reader_map_.end() ) {
        const auto& clsid = it->second;
        auto factory = reader_factories_.find( clsid );
        if ( factory != reader_factories_.end() )
            return factory->second( traceid );
    }
    return nullptr;
}

