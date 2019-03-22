// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include <adacquire/constants.hpp>
#include <boost/any.hpp>
#include <functional>
#include <utility>
#include <vector>
#include <memory>

namespace boost { namespace uuids { struct uuid; } }
namespace adfs { class filesystem; }

namespace adcontrols {

    class DataInterpreter;
    class datafile;
    class Chromatogram;
    class DataReader;
    class DataReader_iterator;
    class MassSpectrum;
    class MassSpectrometer;
    class MappedSpectra;

    class ADCONTROLSSHARED_EXPORT DataReader_value_type {
        std::weak_ptr< const DataReader > reader_;
        int64_t rowid_;
        friend class DataReader_iterator;
    public:
        DataReader_value_type( const DataReader_value_type& t );
        DataReader_value_type( DataReader_iterator * it, int64_t rowid = (-1) );
        DataReader_value_type& operator = ( const DataReader_value_type& t );

        int64_t rowid() const;
        int64_t pos() const;
        int64_t elapsed_time() const;
        double time_since_inject() const;
        int fcn() const;
    };

    class ADCONTROLSSHARED_EXPORT DataReader_iterator
        : public std::iterator< std::bidirectional_iterator_tag
                                , DataReader_iterator > {
        std::weak_ptr< const DataReader > reader_;
        DataReader_value_type value_;
        int fcn_;
    public:
        DataReader_iterator();
        DataReader_iterator( const DataReader* reader, int64_t rowid, int fcn = (-1) );
        DataReader_iterator( const DataReader_iterator& );
        DataReader_iterator& operator = ( const DataReader_iterator& );

        const DataReader_value_type& operator * () const { return value_; };
        const DataReader_value_type* operator -> () const { return &value_; };

        const DataReader_iterator& operator ++ ();
        const DataReader_iterator operator ++ ( int );
        const DataReader_iterator& operator -- ();
        const DataReader_iterator operator -- ( int );
        bool operator == ( const DataReader_iterator& rhs ) const { return value_.rowid() == rhs.value_.rowid(); }
        bool operator != ( const DataReader_iterator& rhs ) const { return value_.rowid() != rhs.value_.rowid(); }

        operator bool() const { return value_.rowid_ != (-1); }

        std::shared_ptr< const DataReader > dataReader() const {
            return reader_.lock();
        }

        inline int64_t rowid() const { return value_.rowid_; }
        inline int _fcn() const { return fcn_; } // query specified fcn
    };

	class ADCONTROLSSHARED_EXPORT DataReader : public std::enable_shared_from_this< DataReader > {
    public:
        virtual ~DataReader(void) {}

        DataReader( const char * traceid = nullptr );

        DataReader( const DataReader& );
        DataReader& operator = ( const DataReader& );

        typedef DataReader_iterator iterator;
        typedef DataReader_iterator const_iterator;

        enum TimeSpec { ElapsedTime, EpochTime };
        enum IndexSpec { TriggerNumber, IndexCount };

        virtual bool initialize( adfs::filesystem&, const boost::uuids::uuid&, const std::string& objtxt ) { return false; }
        virtual void finalize() { return ; }
        virtual size_t fcnCount() const { return 0; }

        virtual const boost::uuids::uuid& objuuid() const = 0;
        virtual const std::string& objtext() const = 0;
        virtual int64_t objrowid() const = 0;                // rowid corresponding to objuuid; this is for backward (v2) compatibility
        virtual const std::string& display_name() const = 0; // return value can be localized

        virtual std::shared_ptr< const adcontrols::Chromatogram > TIC( int fcn ) const { return nullptr; } // eTRACE_SPECTRA (MassSpectra) may return this

        virtual const_iterator begin( int fcn = (-1) ) const = 0;
        virtual const_iterator end() const = 0;

        virtual size_t size( int fcn = (-1) ) const = 0;

        /* findPos returns trigger number on the data stream across all protocol functions */
        virtual const_iterator findPos( double seconds, int fcn = (-1), bool closest = false, TimeSpec ts = ElapsedTime ) const = 0;

        /* findTime returns elapsed time for the data specified by trigger number */
        virtual double findTime( int64_t tpos, IndexSpec ispec = TriggerNumber, bool exactMatch = true ) const = 0;

        enum findPosFlags { findPosRight, findPosClosest };
        static const_iterator findPos( double seconds, const std::vector< std::shared_ptr< const DataReader > >&, findPosFlags = findPosRight );

        // Iterator reference methods
        virtual int64_t next( int64_t rowid ) const { return -1; }
        virtual int64_t next( int64_t rowid, int fcn ) const { return -1; }
        virtual int64_t prev( int64_t rowid ) const { return -1; }
        virtual int64_t prev( int64_t rowid, int fcn ) const { return -1; }
        virtual int64_t pos( int64_t rowid ) const { return -1; }
        virtual int64_t elapsed_time( int64_t rowid ) const { return -1; }
        virtual double time_since_inject( int64_t rowid ) const { return -1; }
        virtual int fcn( int64_t rowid ) const { return -1; }

        virtual boost::any getData( int64_t rowid ) const { return nullptr; }

        virtual std::shared_ptr< adcontrols::MappedSpectra > getMappedSpectra( int64_t rowid ) const { return nullptr; }

        virtual std::shared_ptr< adcontrols::MassSpectrum >  getSpectrum( int64_t rowid ) const { return nullptr; }

        /**
         * @brief Return a chromatogram from a series of spectra computed by time-of-flight and time-width
         */
        virtual std::shared_ptr< adcontrols::Chromatogram >  getChromatogram( int fcn, double time, double width ) const { return nullptr; }

        virtual std::shared_ptr< adcontrols::MassSpectrum >  readSpectrum( const_iterator& it ) const { return nullptr; }

        virtual std::shared_ptr< adcontrols::MassSpectrum >  coaddSpectrum( const_iterator&& begin, const_iterator&& end ) const { return nullptr; }

        virtual std::shared_ptr< adcontrols::MassSpectrometer > massSpectrometer() const { return nullptr; }

        /**
         * @brief Return a chromatogram from a eTRACE_TRACE data
         */
        virtual std::shared_ptr< adcontrols::Chromatogram >  getChromatogram( int idx ) const { return nullptr; }

        virtual DataInterpreter * dataInterpreter() const { return nullptr; }

        virtual const void * _narrow_workaround( const char * /* typename */ ) const { return nullptr; }

        template< typename T > T * _narrow() const {
            return reinterpret_cast< T * >( _narrow_workaround( typeid( T ).name() ) );
        }
        ////////////////////////////////////////
        // SignalObserver::Description
        adacquire::SignalObserver::eTRACE_METHOD trace_method() const;
        const std::string& trace_id() const;
        const std::string& trace_display_name() const;
        std::pair< std::string, std::string > axis_labels() const;
        std::pair< int, int > axis_decimals() const;

        void setDescription(adacquire::SignalObserver::eTRACE_METHOD
                            , const std::string& trace_id
                            , const std::string& trace_display_name
                            , const std::string& axisX_label
                            , const std::string& axisY_label
                            , int axisX_decimals, int axisY_decimals);
    public:
        //////////////////////////////////////////////////////////////
        // singleton interfaces
        typedef std::shared_ptr< DataReader >( factory_type )( const char * traceid );

        static std::shared_ptr< DataReader > make_reader( const char * traceid );

        static void register_factory( std::function< factory_type >, const char * clsid );

        static void assign_reader( const char * clsid, const char * traceid );

    private:
        class impl;
        adacquire::SignalObserver::eTRACE_METHOD trace_method_;
        std::string trace_id_;
        std::string trace_display_name_;
        std::string axisX_label_;
        std::string axisY_label_;
        int32_t axisX_decimals_;
        int32_t axisY_decimals_;
    };

}
