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

#pragma once

#include "adcontrols_global.h"
#include <string>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSReference {
    public:
        MSReference();
        MSReference( const MSReference& t );
        MSReference( const std::wstring& formula
                   , bool polarityPositive
                   , const std::wstring& adduct_or_loss
                   , bool enable = true
                   , double exactMass = 0
                   , const std::wstring& description = L"" );

        bool operator < ( const MSReference& ) const;

        bool enable() const;
        double exactMass() const;
        bool polarityPositive() const;
        const std::wstring& formula() const;
        const std::wstring& adduct_or_loss() const;
        const std::wstring& description() const;

        void enable( bool );
        void exactMass( double );
        void polarityPositive( bool );
        void formula( const std::wstring& );
        void adduct_or_loss( const std::wstring& );
        void description( const std::wstring& );

    private:
        bool enable_;
        double exactMass_;
        bool polarityPositive_;
        size_t chargeCount_;
        std::wstring formula_;
        std::wstring adduct_or_loss_;
        std::wstring description_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            using namespace boost::serialization;
	    (void)version;
	    ar & BOOST_SERIALIZATION_NVP(enable_);
	    ar & BOOST_SERIALIZATION_NVP(exactMass_);
	    ar & BOOST_SERIALIZATION_NVP(polarityPositive_);
	    ar & BOOST_SERIALIZATION_NVP(formula_);
	    ar & BOOST_SERIALIZATION_NVP(adduct_or_loss_);
	    ar & BOOST_SERIALIZATION_NVP(description_);
        }

    };

}


