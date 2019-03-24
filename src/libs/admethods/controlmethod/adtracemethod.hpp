// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC
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

#include "../admethods_global.hpp"
#include <array>
#include <string>
#include <map>
#include <vector>
#include <boost/uuid/uuid.hpp>

namespace boost { namespace serialization { class access; } }

namespace admethods {
    namespace controlmethod {

        class ADMETHODSSHARED_EXPORT ADTraceMethod;
        class ADMETHODSSHARED_EXPORT ADTrace;

        class ADTrace {
        public:
            ADTrace();
            ADTrace( const ADTrace& );
            ADTrace( std::tuple< bool, std::string, double >&& t );

            bool enable() const;
            void setEnable( bool );

            double vOffset() const;
            void setVOffset( double );

            std::string legend() const;
            void setLegend( const std::string& );
        private:
            std::tuple< bool, std::string, double > d_;  // enable, legend, vOffset
        };

        ////////////////////////////////////////

        class ADTraceMethod {
        public:
            ~ADTraceMethod();
            ADTraceMethod();
            ADTraceMethod( const ADTraceMethod& );

            static constexpr const char * const __modelClass__ = "admethods::cm::adTracesMethods"; // DE0 LTC2301 8ch 500ksps 12bit adc
            static constexpr const char * const __clsid_str    = "3ba45f4c-c46e-4ef7-8964-02cace4bbd4f";
            static constexpr boost::uuids::uuid __clsid__      = {{ 0x3b, 0xa4, 0x5f, 0x4c, 0xc4, 0x6e, 0x4e, 0xf7, 0x89, 0x64, 0x02, 0xca, 0xce, 0x4b, 0xbd, 0x4f }};

            static const boost::uuids::uuid& clsid() { return __clsid__; }
            static const char * modelClass() { return __modelClass__; }
            static const char * itemLabel() { return "ADTraces.1"; }

            size_t size() const { return 8; }
            const ADTrace& operator [] ( size_t idx ) const { return data_[ idx ]; }
            ADTrace& operator [] ( size_t idx ) { return data_[ idx ]; }
            const std::array< ADTrace, 8 >::const_iterator begin() { return data_.begin(); }
            const std::array< ADTrace, 8 >::const_iterator end() { return data_.end(); }

            std::string toJson( bool pritty = false ) const;
            void fromJson( const std::string& );

            static bool archive( std::ostream&, const ADTraceMethod& );
            static bool restore( std::istream&, ADTraceMethod& );
        private:
            std::array< ADTrace, 8 > data_;
            friend class boost::serialization::access;
            template< class Archive > void serialize( Archive&, const unsigned int );
        };
    }
}
