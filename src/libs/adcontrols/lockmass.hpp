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

#ifndef LOCKMASS_H
#define LOCKMASS_H

#include "adcontrols_global.h"
#include <string>
#include <vector>
#include <cstdint>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <compiler/diagnostic_push.h>

namespace adcontrols {

    class MassSpectrum;
    class MSPeakInfo;

    class ADCONTROLSSHARED_EXPORT lockmass {
    public:
        lockmass();
        lockmass( const lockmass& );
        operator bool () const; // true if at least one reference

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

        struct ADCONTROLSSHARED_EXPORT fitter {
            std::vector< double > coeffs_;
            bool operator()( MassSpectrum& ) const; // correct mass array
            bool operator()( MSPeakInfo& ) const;
            void clear();
        private:
            friend class boost::serialization::access;
            template<class Archive>
                void serialize(Archive& ar, const unsigned int ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP(coeffs_)
                    ;
            }
        };

        lockmass& operator << ( const reference& );
        void clear();
        size_t size() const;
        bool empty() const;

        static bool findReferences( lockmass&,  const adcontrols::MassSpectrum& );
        static bool findReferences( lockmass&,  const adcontrols::MassSpectrum&, int idx, int fcn );
        bool fit();
        bool operator()( MassSpectrum&, bool applyToAll = true ) const; // correct mass array
        bool operator()( MSPeakInfo&, bool applyToAll = true ) const;
        typedef std::vector< reference >::const_iterator const_iterator;
        const_iterator begin() const;
        const_iterator end() const;
        
    private:
        std::vector< reference > references_;
        fitter fitter_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize(Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP(references_)
                & BOOST_SERIALIZATION_NVP(fitter_)
                ;
        }

    };

#if defined _MSC_VER
    template class ADCONTROLSSHARED_EXPORT std::vector < lockmass::reference > ;
#endif
}

#endif // LOCKMASS_H
