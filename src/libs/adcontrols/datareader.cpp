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
#include <boost/uuid/uuid.hpp>
#include <cmath>
#include <limits>
#include <map>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

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

    class NullDataReader : public DataReader {
        boost::uuids::uuid uuid_;
        std::string objtext_;
    public:
        NullDataReader() : uuid_( { {0} } ), objtext_("null_reader") {}
        const boost::uuids::uuid& objuuid() const override { return uuid_; }
        const std::string& objtext() const override { return objtext_; }
        int64_t objrowid() const override { return 0; }
        const std::string& display_name() const override { return objtext_; }
        const_iterator begin( int fcn ) const override { return end(); }
        const_iterator end() const override { return const_iterator(this, -1); }
        const_iterator findPos( double seconds, int fcn, bool closest = false, TimeSpec ts = ElapsedTime ) const override { return end(); }
        double findTime( int64_t tpos, IndexSpec ispec = TriggerNumber, bool exactMatch = true ) const override { return end(); }
        
        static std::shared_ptr< DataReader > instance() {

            static std::shared_ptr< NullDataReader >  __instance;

            static std::once_flag flag;
            std::call_once( flag, [&](){ __instance = std::make_shared< NullDataReader >(); } );
                            
            return __instance;
        }
    };

}

using namespace adcontrols;

DataReader_value_type::DataReader_value_type( const DataReader_value_type& t ) : reader_( t.reader_ )
                                                                               , rowid_( t.rowid_ )
{
}

DataReader_value_type::DataReader_value_type( DataReader_iterator * it, int64_t rowid ) : reader_( it->dataReader() )
                                                                                        , rowid_( rowid )
{
}

DataReader_value_type&
DataReader_value_type::operator = ( const DataReader_value_type& t )
{
    reader_ = t.reader_;
    rowid_ = t.rowid_;
    return *this;
}

/////////////

DataReader_iterator::DataReader_iterator() : reader_( NullDataReader::instance() )
                                           , value_( this )
{
}

DataReader_iterator::DataReader_iterator( const DataReader* reader
                                          , int64_t rowid
                                          , int fcn ) : reader_( reader->shared_from_this() )
                                                      , value_( this, rowid )
                                                      , fcn_( fcn )
{
}

DataReader_iterator::DataReader_iterator( const DataReader_iterator& t ) : reader_( t.reader_ )
                                                                         , value_( this, t.value_.rowid_ )
                                                                         , fcn_( t.fcn_ )
{
}

DataReader_iterator&
DataReader_iterator::operator = ( const DataReader_iterator& t ) 
{
    reader_ = t.reader_;
    value_ = t.value_;
    fcn_ = t.fcn_;
    return *this;
}

const DataReader_iterator&
DataReader_iterator::operator ++ ()
{
    if ( auto reader = reader_.lock() ) {
        value_.rowid_ = reader->next( value_.rowid_, fcn_ );
    }
    return *this;
}

const DataReader_iterator
DataReader_iterator::operator ++ ( int )
{
    DataReader_iterator temp( *this );

    if ( auto reader = reader_.lock() )
        value_.rowid_ = reader->next( value_.rowid_, fcn_ );

    return temp;
}

const DataReader_iterator&
DataReader_iterator::operator -- ()
{
    if ( auto reader = reader_.lock() ) {
        value_.rowid_ = reader->prev( value_.rowid_, fcn_ );
    }
    return *this;
}

const DataReader_iterator
DataReader_iterator::operator -- ( int )
{
    DataReader_iterator temp( *this );

    if ( auto reader = reader_.lock() )
        value_.rowid_ = reader->prev( value_.rowid_, fcn_ );

    return temp;
}

///////////////////////////////////////////////////////
int64_t
DataReader_value_type::rowid() const
{
    return rowid_;
}

int64_t
DataReader_value_type::pos() const
{
    if ( auto reader = reader_.lock() )
        return reader->pos( rowid_ );
    return (-1);
}

int64_t
DataReader_value_type::elapsed_time() const
{
    if ( auto reader = reader_.lock() )
        return reader->elapsed_time( rowid_ );
    return (-1);    
}

double
DataReader_value_type::time_since_inject() const
{
    if ( auto reader = reader_.lock() )
        return reader->time_since_inject( rowid_ );
    return (-1);
}

int
DataReader_value_type::fcn() const
{
    if ( auto reader = reader_.lock() )
        return reader->fcn( rowid_ );
    return (-1);
}

/////////////////////

DataReader::DataReader( const char * traceid )
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

//////////////////////////

//static
DataReader::const_iterator 
DataReader::findPos( double seconds, const std::vector< std::shared_ptr< const DataReader > >& readers, findPosFlags flag )
{
    double diff = std::numeric_limits<double>::max();
    iterator result;

    for ( auto& reader : readers ) {
        if ( auto it = reader->findPos( seconds ) ) {
            double tdiff = std::abs( it->time_since_inject() - seconds );
            if ( diff > tdiff ) {
                diff = tdiff;
                result = it;
            }
        }
    }
    return result;
}
