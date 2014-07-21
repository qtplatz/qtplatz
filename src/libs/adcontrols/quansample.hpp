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
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <memory>
#include <vector>

namespace adcontrols {

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

        struct ADCONTROLSSHARED_EXPORT Response {
            Response() : identified_( false ), intensity_(0), amounts_(0) {}
            bool identified_;
            double intensity_;
            double amounts_;
        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( status_ )
                    & BOOST_SERIALIZATION_NVP( intensity_ )
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

        enum QuanSampleType {
            SAMPLE_TYPE_UNKNOWN
            , SAMPLE_TYPE_STD
            , SAMPLE_TYPE_QC
            , SAMPLE_TYPE_BLANK
        };

        const wchar_t * name() const;
        void name( const wchar_t * );

        const wchar_t * dataSource() const;
        void dataSource( const wchar_t * );

        const wchar_t * dataGuid() const;
        void dataGuid( const wchar_t * );

        QuanSampleType sampleType() const;
        void sampleType( QuanSampleType );
        
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

    private:
#     include <compiler/diagnostic_push.h>
#     include <compiler/disable_dll_interface.h>
        std::wstring name_;
        std::wstring dataType_;
        std::wstring dataSource_;             // fullpath for data file + "::" + data node
        std::wstring dataGuid_;               // data guid on portfolio (for redisplay)
#     include <compiler/diagnostic_pop.h>
        QuanSampleType sampleType_; 
        int32_t level_;                       // 0 for UNK, otherwise >= 1
        int32_t istdId_;                      // id for istd sample (id for myself if this is ISTD)
        double injVol_;                       // conc. for infusion
        double amountsAdded_;                 // added amount for standard
        std::vector< quan::ISTD > istd_;      // index is correspoinding to ISTD id
        quan::Response quanResult_;           // result

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( name_ )
                & BOOST_SERIALIZATION_NVP( dataType_ )
                & BOOST_SERIALIZATION_NVP( dataSource_ )
                & BOOST_SERIALIZATION_NVP( dataGuid_ )
                & BOOST_SERIALIZATION_NVP( sampleType_ )
                & BOOST_SERIALIZATION_NVP( level_ )
                & BOOST_SERIALIZATION_NVP( istdId_ )
                & BOOST_SERIALIZATION_NVP( injVol_ )
                & BOOST_SERIALIZATION_NVP( amountAdded_ )
                & BOOST_SERIALIZATION_NVP( istd_ )
                & BOOST_SERIALIZATION_NVP( quanResult_ )
                ;
        }
    };
    
}

BOOST_CLASS_VERSION( adcontrols::QuanSample, 1 )
BOOST_CLASS_VERSION( adcontrols::quan::ISTD, 1 )
BOOST_CLASS_VERSION( adcontrols::quan::Response, 1 )

#endif // QUANSAMPLE_HPP
