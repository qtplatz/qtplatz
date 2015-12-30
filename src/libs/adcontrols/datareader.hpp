// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include "adcontrols_global.h"
#include <functional>
#include <utility>
#include <memory>

namespace boost { namespace uuids { struct uuid; } }
namespace adfs { class filesystem; }

namespace adcontrols {

    class DataInterpreter;
    class datafile;
    class Chromatogram;

    class ADCONTROLSSHARED_EXPORT DataReader_index {
    public:
        virtual int64_t pos() const = 0;
        virtual int64_t elapsed_time() const = 0;
        virtual double time_since_inject() const = 0;
        virtual void operator ++() = 0;
        virtual bool operator == ( DataReader_index& t ) const = 0;
        virtual bool operator != ( DataReader_index& t ) const = 0;
    };

    template< typename T = DataReader_index >
    class ADCONTROLSSHARED_EXPORT DataReader_iterator {
    protected:
        std::unique_ptr<T> index_;
    public:
        virtual ~DataReader_iterator() {}

        DataReader_iterator( std::unique_ptr<T>&& i ) : index_( std::move(i) ) {}
        DataReader_iterator( DataReader_iterator<T>&& t ) : index_( std::move(t.index_) ) {}
        DataReader_iterator& operator = ( const DataReader_iterator&& t ) { index_ = std::move( t.index_ ); return * this; }
        DataReader_iterator& operator = ( const DataReader_iterator& t ) { index_.reset( t.index_.release() ); return * this; }
        
        virtual T& operator * () const { return *index_; }
        virtual T* operator -> () const { return index_.get(); }
        virtual const T& operator ++ () { ++(*index_); }

        virtual bool operator == ( const DataReader_iterator<T>& t ) const { return (*index_) == (*t.index_); }
        virtual bool operator != ( const DataReader_iterator<T>& t ) const { return (*index_) != (*t.index_); }
    };

	class ADCONTROLSSHARED_EXPORT DataReader {

        DataReader( const DataReader& ) = delete;  // noncopyable
        DataReader& operator = (const DataReader&) = delete;

    public:
        virtual ~DataReader(void);
        DataReader( const char * traceid = nullptr );
        DataReader( adfs::filesystem&, const char * traceid = nullptr );

        typedef DataReader_iterator<DataReader_index> iterator;
        typedef const DataReader_iterator<DataReader_index> const_iterator;
        typedef DataReader_index value_type;

        enum TimeSpec { ElapsedTime, EpochTime };
        enum IndexSpec { TriggerNumber, IndexCount };

        virtual bool initialize( adfs::filesystem&, const boost::uuids::uuid&, const std::string& objtxt = "" ) { return false; }
        virtual void finalize() { return ; }
        virtual size_t fcnCount() const { return 0; }
        virtual std::shared_ptr< const adcontrols::Chromatogram > TIC( int fcn ) const { return nullptr; }

        virtual const_iterator begin() const = 0;
        virtual const_iterator end() const = 0;
        
        /* findPos returns trigger number on the data stream across all protocol functions */
        virtual const_iterator findPos( double seconds, bool closest = false, TimeSpec ts = ElapsedTime ) const = 0;

        /* findTime returns elapsed time for the data specified by trigger number */
        virtual double findTime( int64_t tpos, IndexSpec ispec = TriggerNumber, bool exactMatch = true ) const = 0;

        //////////////////////////////////////////////////////////////
        // singleton interfaces
        typedef std::shared_ptr< DataReader >( factory_type )( const char * traceid );
        
        static std::shared_ptr< DataReader > make_reader( const char * traceid );

        static void register_factory( std::function< factory_type >, const char * clsid );

        static void assign_reader( const char * clsid, const char * traceid );

    private:
        class impl;

    };

}

