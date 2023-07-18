// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace adcontrols {

    class MassSpectrum;
    class MSPeakInfo;

    namespace lockmass {

        class ADCONTROLSSHARED_EXPORT reference;
        class ADCONTROLSSHARED_EXPORT fitter;
        class ADCONTROLSSHARED_EXPORT mslock;

        ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const reference& );
        ADCONTROLSSHARED_EXPORT reference tag_invoke( boost::json::value_to_tag< reference >&, const boost::json::value& jv );

        ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const fitter& );
        ADCONTROLSSHARED_EXPORT fitter tag_invoke( boost::json::value_to_tag< fitter >&, const boost::json::value& jv );

        ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const mslock& );
        ADCONTROLSSHARED_EXPORT mslock tag_invoke( boost::json::value_to_tag< mslock >&, const boost::json::value& jv );


        class ADCONTROLSSHARED_EXPORT reference {
        public:
            reference();
            reference( const std::string& formula, double exactMass, double mass, double time );
            reference( const reference& );
            const std::string& formula() const;
            double exactMass() const;
            double matchedMass() const;
            double time() const;

        private:
            std::string formula_;
            double exactMass_;
            double matchedMass_;
            double time_;
            friend ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const reference& );
            friend ADCONTROLSSHARED_EXPORT reference tag_invoke( boost::json::value_to_tag< reference >&, const boost::json::value& );
            friend class boost::serialization::access;
            template<class Archive>
                void serialize(Archive& ar, const unsigned int ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP(formula_)
                    & BOOST_SERIALIZATION_NVP(exactMass_)
                    & BOOST_SERIALIZATION_NVP(matchedMass_) // result
                    & BOOST_SERIALIZATION_NVP(time_)        // result
                    ;
            }
        };

        /////////////////////
        /////////////////////
        class ADCONTROLSSHARED_EXPORT fitter {
        public:
            fitter();
            fitter( const fitter& );
            fitter( std::vector< double >&& );
            fitter( const std::vector< double >& );
            fitter( const std::array<double, 2 >& );
            fitter& operator = ( std::vector< double >&& );
            bool operator()( MassSpectrum& ) const; // correct mass array
            bool operator()( MSPeakInfo& ) const;
            void clear();
            const std::vector< double >& coeffs() const;
            std::vector< double >& coeffs();

        private:
            std::vector< double > coeffs_;

            friend ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const fitter& );
            friend ADCONTROLSSHARED_EXPORT fitter tag_invoke( boost::json::value_to_tag< fitter >&, const boost::json::value& );
            friend class boost::serialization::access;
            template<class Archive>
                void serialize(Archive& ar, const unsigned int ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP(coeffs_)
                    ;
            }
        };

        /////////////////////
        /////////////////////
        class ADCONTROLSSHARED_EXPORT mslock {
        public:
            mslock();
            mslock( const mslock& );
            operator bool () const; // true if at least one reference
            mslock& operator << ( const reference& );
            mslock& operator += ( const mslock& );
            void clear();
            size_t size() const;
            bool empty() const;

            static bool findReferences( mslock&,  const adcontrols::MassSpectrum& );
            static bool findReferences( mslock&,  const adcontrols::MassSpectrum&, int idx, int fcn );
            bool fit();
            bool operator()( MassSpectrum&, bool applyToAll = true ) const; // correct mass array
            bool operator()( MSPeakInfo&, bool applyToAll = true ) const;
            typedef std::vector< reference >::const_iterator const_iterator;
            const_iterator begin() const;
            const_iterator end() const;
            const std::vector< double >& coeffs() const;
            const lockmass::fitter& fitter() const;

            int64_t posix_time() const;

            std::optional< std::string > property() const;
            void setProperty( std::pair< std::string, std::string >&& keyValue );

            static constexpr const wchar_t * const __dataClass__ = L"adcontrols::lockmass::mslock";
            static const wchar_t * const dataClass() { return __dataClass__; }
            static bool archive( std::ostream&, const mslock& );
            static bool restore( std::istream&, mslock& );

        private:
            std::vector< reference > references_;
            lockmass::fitter fitter_;
            int64_t posix_time_;
            std::string property_;

            friend ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const mslock& );
            friend ADCONTROLSSHARED_EXPORT mslock tag_invoke( boost::json::value_to_tag< mslock >&, const boost::json::value& );

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP(references_)
                    & BOOST_SERIALIZATION_NVP(fitter_)
                    ;
                if ( version >= 1 ) {
                    ar & posix_time_;
                    ar & property_;
                }
            }
        };

#if defined _MSC_VER
        template class ADCONTROLSSHARED_EXPORT std::vector < reference > ;
#endif
    }
}

BOOST_CLASS_VERSION( adcontrols::lockmass::mslock, 1 )
