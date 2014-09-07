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
#include <memory>
#include <cstdint>
#include <boost/serialization/version.hpp>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSReference {
    public:
        ~MSReference();
        MSReference();
        MSReference( const MSReference& t );
        MSReference( const wchar_t * formula
                   , bool polarityPositive
                   , const wchar_t * adduct_or_loss
                   , bool enable = true
                   , double exactMass = 0
				   , uint32_t charge = 1
                   , const wchar_t * description = L"" );

        MSReference& operator = ( const MSReference& );
        bool operator < ( const MSReference& ) const;

        bool enable() const;
        double exact_mass() const;
        bool polarityPositive() const;
		uint32_t charge_count() const;
        const wchar_t * formula() const;
        const wchar_t * adduct_or_loss() const;
        const wchar_t * display_formula() const;
        const wchar_t * description() const;

        void enable( bool );
        void exact_mass( double );
        void polarityPositive( bool );
		void charge_count( uint32_t );
        void formula( const wchar_t * );
        void formula( const char * );
        void adduct_or_loss( const wchar_t * );
        void adduct_or_loss( const char * );
        void description( const wchar_t * );

    private:

#   if  defined _MSC_VER
#   pragma warning(disable:4251)
#   endif
        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive>  void serialize( Archive& ar, const unsigned int version );
    };

}

BOOST_CLASS_VERSION( adcontrols::MSReference, 1 )
