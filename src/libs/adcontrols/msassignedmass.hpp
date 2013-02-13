// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#ifndef MSASSIGNEDMASS_H
#define MSASSIGNEDMASS_H

#include "adcontrols_global.h"
#include <string>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSAssignedMass {
    public:
        MSAssignedMass();
        MSAssignedMass( const MSAssignedMass& );
        
        MSAssignedMass( unsigned int idReferences, unsigned int idMasSpectrum
                        , const std::wstring& formula, double exactMass, double time, double mass
                        , bool enable, unsigned int flags = 0 );
        const std::wstring& formula() const;
        unsigned int idReferences() const;
        unsigned int idMassSpectrum() const;
        double exactMass() const;
        double time() const;
        double mass() const;
        bool enable() const;
        unsigned int flags() const;
        void formula( const std::wstring& );
        void idReferences( unsigned int );
        void idMassSpectrum( unsigned int );
        void exactMass( double );
        void time( double );
        void mass( double );
        void enable( bool );
        void flags( unsigned int );
 
    private:
        std::wstring formula_;
        unsigned int idReferences_;
        unsigned int idMassSpectrum_;
        double exactMass_;
        double time_;
        double mass_;
        bool enable_;  // if false, this peak does not use for polynomial fitting
        unsigned int flags_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP(formula_);
            ar & BOOST_SERIALIZATION_NVP(idReferences_);
            ar & BOOST_SERIALIZATION_NVP(idMassSpectrum_);
            ar & BOOST_SERIALIZATION_NVP(exactMass_);
            ar & BOOST_SERIALIZATION_NVP(time_);
            ar & BOOST_SERIALIZATION_NVP(mass_);
            ar & BOOST_SERIALIZATION_NVP(enable_);
            if ( version >= 1 )
                ar & BOOST_SERIALIZATION_NVP( flags_ );
        }

    };


    class ADCONTROLSSHARED_EXPORT MSAssignedMasses {
    public:
        MSAssignedMasses();
        MSAssignedMasses( const MSAssignedMasses& );
        typedef std::vector< MSAssignedMass > vector_type;

        size_t size() const;
        vector_type::iterator begin();
        vector_type::iterator end();
        vector_type::const_iterator begin() const;
        vector_type::const_iterator end() const;
        MSAssignedMasses& operator << ( const MSAssignedMass& );

    private:
# if defined _MSC_VER
#  pragma warning( disable: 4251 )
# endif
        std::vector< MSAssignedMass > vec_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int /*version*/) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP(vec_);
        }

    };


}

BOOST_CLASS_VERSION( adcontrols::MSAssignedMass, 1 )

#endif // MSASSIGNEDMASS_H
