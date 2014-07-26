/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANSAMPLE_HPP
#define QUANSAMPLE_HPP

#include "adcontrols_global.h"
#include "quanresponses.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/utility.hpp>
#include <cstdint>
#include <memory>
#include <vector>
#include <compiler/disable_dll_interface.h>
#include <boost/uuid/uuid.hpp>

namespace adcontrols {

    class QuanResponse;

    namespace quan {

        struct ADCONTROLSSHARED_EXPORT ISTD {
            ISTD() : id_(0), amounts_(0) {}
            ISTD( uint32_t id, double a ) : id_( id ), amounts_( a ) {}
            uint32_t id_;
            double amounts_;
        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( id_ )
                    & BOOST_SERIALIZATION_NVP( amounts_ )
                    ;
            }
        };

#if defined _MSC_VER
        template class ADCONTROLSSHARED_EXPORT std::vector < quan::ISTD > ;
#endif
    }

    class ADCONTROLSSHARED_EXPORT QuanSample {
    public:
        ~QuanSample();
        QuanSample();
        QuanSample( const QuanSample& );
        static const wchar_t * dataClass() { return L"adcontrols::QuanSample"; }
        
        enum QuanSampleType {
            SAMPLE_TYPE_UNKNOWN
            , SAMPLE_TYPE_STD
            , SAMPLE_TYPE_QC
            , SAMPLE_TYPE_BLANK
        };
        
        enum QuanInlet {
            Chromatography
            , Infusion
        };

        enum QuanDataGeneration {
            ASIS
            , GenerateSpectrum
            , GenerateChromatogram
        };

        const boost::uuids::uuid& sequence_uuid() const { return sequence_uuid_; }
        const int32_t row() const { return rowid_; }
        void sequence_uuid( boost::uuids::uuid& d, int32_t rowid ) { sequence_uuid_ = d; rowid_ = rowid; }

        const wchar_t * name() const;
        void name( const wchar_t * );

        const wchar_t * dataSource() const;
        void dataSource( const wchar_t * );

        const wchar_t * dataGuid() const;
        void dataGuid( const wchar_t * );

        const wchar_t * dataType() const { return dataType_.c_str(); }
        void dataType( const wchar_t * v ) { dataType_ = v; }

        QuanSampleType sampleType() const;
        void sampleType( QuanSampleType );

        QuanDataGeneration dataGeneration() const { return dataGeneration_; }
        void dataGeneration( QuanDataGeneration v ) { dataGeneration_ = v; }
        uint32_t scan_range_first() const { return scan_range_.first; }
        uint32_t scan_range_second() const { return scan_range_.second; }
        void scan_range( int32_t first, int32_t second ) { scan_range_ = std::make_pair( first, second ); }

        int32_t channel() const  { return channel_; }
        void channel( int32_t t ) { channel_ = t; }
        
        int32_t istdId() const;
        void istdId( int32_t );
        
        int32_t level() const;
        void level( int32_t );
        
        double injVol() const;  // ignore when infusion
        void injVol( double );  // ignore when infusion
        
        double addedAmounts() const;
        void addedAmounts( double );

        const std::vector< quan::ISTD >& istd() const;
        void istd( const std::vector< quan::ISTD >& );

        QuanSample& operator << ( const quan::ISTD& );
        QuanSample& operator << (const QuanResponse&);

        const QuanResponses& results() const { return results_; }

        static bool archive( std::ostream&, const QuanSample& );
        static bool restore( std::istream&, QuanSample& );

    private:
        boost::uuids::uuid sequence_uuid_;       // points to parement
        int32_t rowid_;                          // row number on sequence
        std::wstring name_;
        std::wstring dataType_;
        std::wstring dataSource_;                // fullpath for data file + "::" + data node
        std::wstring dataGuid_;                  // data guid on portfolio (for redisplay)
        QuanSampleType sampleType_;
        QuanInlet inletType_;                    // Infusion | Chromatogram
        int32_t level_;                          // 0 for UNK, otherwise >= 1
        int32_t istdId_;                         // id for istd sample (id for myself if this is ISTD)
        double injVol_;                          // conc. for infusion
        double amountsAdded_;                    // added amount for standard
        std::vector< quan::ISTD > istd_;         // index is correspoinding to ISTD id
        QuanResponses results_;
        QuanDataGeneration dataGeneration_;
        std::pair<int32_t,int32_t> scan_range_; // 0 := first spectrum, 1 := second spectrum, -1 := last spectrum
        int32_t channel_;                       // quan protocol id (channel

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( sequence_uuid_ )
                & BOOST_SERIALIZATION_NVP( rowid_ )
                & BOOST_SERIALIZATION_NVP( name_ )
                & BOOST_SERIALIZATION_NVP( dataSource_ )
                & BOOST_SERIALIZATION_NVP( dataType_ )
                & BOOST_SERIALIZATION_NVP( dataGuid_ )
                & BOOST_SERIALIZATION_NVP( sampleType_ )
                & BOOST_SERIALIZATION_NVP( level_ )
                & BOOST_SERIALIZATION_NVP( istdId_ )
                & BOOST_SERIALIZATION_NVP( injVol_ )
                & BOOST_SERIALIZATION_NVP( amountsAdded_ )
                & BOOST_SERIALIZATION_NVP( istd_ )
                & BOOST_SERIALIZATION_NVP( dataGeneration_ )
                & BOOST_SERIALIZATION_NVP( scan_range_ )
                & BOOST_SERIALIZATION_NVP( channel_ )
                & BOOST_SERIALIZATION_NVP( results_ )
                ;
        }
    };
    
}

BOOST_CLASS_VERSION( adcontrols::QuanSample, 1 )
BOOST_CLASS_VERSION( adcontrols::quan::ISTD, 1 )

#endif // QUANSAMPLE_HPP
