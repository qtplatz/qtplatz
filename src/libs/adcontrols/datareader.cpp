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
        const_iterator begin() const override { return end(); }
        const_iterator end() const override { return const_iterator(this, -1); }
        const_iterator findPos( double seconds, bool closest = false, TimeSpec ts = ElapsedTime ) const override { return end(); }
        double findTime( int64_t tpos, IndexSpec ispec = TriggerNumber, bool exactMatch = true ) const override { return end(); }

        static DataReader& instance() {
            static NullDataReader __instance;
            return __instance;
        }
    };

}

using namespace adcontrols;

DataReader_value_type::DataReader_value_type( DataReader_iterator * it ) : iterator_( it )
                                                                         , rowid_( it->rowid() )
{
}

#if 0
DataReader_value_type::DataReader_value_type( const DataReader_value_type& t ) : iterator_( t.iterator_ )
                                                                               , rowid_( t.rowid() )
{
}
#endif

/////////////

DataReader_iterator::DataReader_iterator() : reader_( &NullDataReader::instance() )
                                           , rowid_( -1 )
                                           , value_( this )
{
}

DataReader_iterator::DataReader_iterator( const DataReader* reader
                                         , int64_t rowid ) : reader_( reader )
                                                           , rowid_( rowid )
                                                           , value_( this )
{
}

DataReader_iterator::DataReader_iterator( const DataReader_iterator& t ) : reader_( t.reader_ )
                                                                         , rowid_( t.rowid_ )
                                                                         , value_( this )
{
}

DataReader_iterator&
DataReader_iterator::operator = ( const DataReader_iterator& t ) 
{
    reader_ = t.reader_;
    rowid_ = t.rowid_;
    value_ = t.value_;
    return *this;
}

const DataReader_iterator&
DataReader_iterator::operator ++ ()
{
    rowid_ = rowid_ = reader_->next( rowid_ );
    return *this;
}

const DataReader_iterator
DataReader_iterator::operator ++ ( int )
{
    DataReader_iterator temp( *this );
    rowid_ = rowid_ = reader_->next( rowid_ );
    return temp;
}

int64_t
DataReader_value_type::rowid() const
{
    return rowid_;
}

int64_t
DataReader_value_type::pos() const
{
    return iterator_->dataReader()->pos( rowid_ );
}

int64_t
DataReader_value_type::elapsed_time() const
{
    return iterator_->dataReader()->elapsed_time( rowid_ );
}

double
DataReader_value_type::time_since_inject() const
{
    return iterator_->dataReader()->time_since_inject( rowid_ );
}

int
DataReader_value_type::fcn() const
{
    return iterator_->dataReader()->fcn( rowid_ );
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

//////////////////////////

//static
DataReader::const_iterator 
DataReader::findPos( double seconds, const std::vector< std::shared_ptr< const DataReader > >& readers, findPosFlags flag )
{
    double diff = std::numeric_limits<double>::max();
    const_iterator result;

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
