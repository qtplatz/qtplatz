// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include "adcontrols_global.h"
#include <string>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSAssignedMass;

    class MSAssignedMass {
    public:
        ~MSAssignedMass();
        MSAssignedMass();
        MSAssignedMass( const MSAssignedMass& );
        const MSAssignedMass& operator = ( const MSAssignedMass& );

        MSAssignedMass( uint32_t idReference
                        , uint32_t idMasSpectrum
                        , uint32_t idPeak
                        , const std::string& formula, double exactMass, double time, double mass
                        , bool enable, uint32_t flags, uint32_t mode );

        std::string formula() const;
        uint32_t idReference() const;
        uint32_t idMassSpectrum() const; // fcn
        uint32_t idPeak() const;  // index on MassSpectrum
        double exactMass() const;
        double time() const;
        double mass() const;
        bool enable() const;
        uint32_t flags() const;
        uint32_t mode() const;
        void formula( const std::wstring& );
        void formula( const std::string& );
        void idReference( uint32_t );
        void idMassSpectrum( uint32_t );
        void idPeak( uint32_t );
        void exactMass( double );
        void time( double );
        void mass( double );
        void enable( bool );
        void flags( uint32_t );
        void mode( uint32_t );

    private:
        class impl;
        std::unique_ptr< impl > impl_;
        std::string formula_;
        uint32_t idReference_;
        uint32_t idMassSpectrum_; // segment# on segment_wrapper<MassSpectrum>[]
        uint32_t idPeak_;         // peak# on MassSpectrum
        double exactMass_;
        double time_;
        double mass_;
        bool enable_;  // if false, this peak does not use for polynomial fitting
        uint32_t flags_;
        uint32_t mode_; // number of turns for InfiTOF, linear|reflectron for MALDI and/or any analyzer mode

        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const uint32_t version);
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
        MSAssignedMasses& operator += ( const MSAssignedMasses& );
        bool operator += ( const MSAssignedMass& );

    private:
        std::vector< MSAssignedMass > vec_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize(Archive& ar, const uint32_t /* version */ ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP(vec_);
        }

    };

}

BOOST_CLASS_VERSION( adcontrols::MSAssignedMass, 4 ) // 2023-04-03
BOOST_CLASS_VERSION( adcontrols::MSAssignedMasses, 1 )
