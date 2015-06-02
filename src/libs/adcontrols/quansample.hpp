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
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <memory>
#include <vector>
#include <compiler/disable_dll_interface.h>
#include <workaround/boost/uuid/uuid.hpp>

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
            template<class Archive> void serialize( Archive& ar, const unsigned int );
        };
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
            , ProcessRawSpectra     // Process each raw spectrum and stored identified ions on db
        };

        const boost::uuids::uuid& sequence_uuid() const;
        int32_t row() const;
        void sequence_uuid( const boost::uuids::uuid& d, int32_t rowid );

        const boost::uuids::uuid& uuid() const;

        const wchar_t * name() const;
        void name( const wchar_t * );

        const wchar_t * dataSource() const;
        void dataSource( const wchar_t * );

        const wchar_t * dataGuid() const;
        void dataGuid( const wchar_t * );

        const wchar_t * description() const;
        void description( const wchar_t * );

        const wchar_t * dataType() const;
        void dataType( const wchar_t * v );

        QuanSampleType sampleType() const;
        void sampleType( QuanSampleType );

        QuanDataGeneration dataGeneration() const;
        void dataGeneration( QuanDataGeneration v );

        uint32_t scan_range_first() const;
        uint32_t scan_range_second() const;

        void scan_range( int32_t first, int32_t second );

        int32_t channel() const;
        void channel( int32_t t );
        
        int32_t istdId() const;
        void istdId( int32_t );
        
        int32_t level() const;
        void level( int32_t );
        
        double injVol() const;  // ignore when infusion
        void injVol( double );  // ignore when infusion
        
        double addedAmounts() const;
        void addedAmounts( double );

        QuanInlet inletType() const;
        void inletType( QuanInlet v );

        const std::vector< quan::ISTD >& istd() const;
        void istd( const std::vector< quan::ISTD >& );

        QuanSample& operator << ( const quan::ISTD& );
        QuanSample& operator << (const QuanResponse&);

        const QuanResponses& results() const;
        QuanResponses& results();

        static bool archive( std::ostream&, const QuanSample& );
        static bool restore( std::istream&, QuanSample& );
        static bool xml_archive( std::wostream&, const QuanSample& );
        static bool xml_restore( std::wistream&, QuanSample& );

    private:

        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

    typedef std::shared_ptr<QuanSample> QuanSamplePtr;   
}

BOOST_CLASS_VERSION( adcontrols::QuanSample, 3 )
BOOST_CLASS_VERSION( adcontrols::quan::ISTD, 1 )

#endif // QUANSAMPLE_HPP
