// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

    class ADCONTROLSSHARED_EXPORT MSRefFormula {
    public:
        ~MSRefFormula();
        MSRefFormula();
        MSRefFormula( const MSRefFormula& );
        MSRefFormula( const std::wstring& formula, bool enable = true, bool positive = true, const std::wstring& adduct = L"", const std::wstring& loss = L"", size_t chargeCount = 1 );

        operator bool () const;
        
        const std::wstring& formula() const;
        const std::wstring& adduct() const;
        const std::wstring& loss() const;
        bool polarityPositive() const;
        size_t chargeCount() const;
        const std::wstring& comments() const;

        void enable( bool );
        void useIsotopes( bool );
        void formula( const std::wstring& );
        void adduct( const std::wstring& );
        void loss( const std::wstring& );
        void polarityPositive( bool );
        void chargeCount( size_t );
        void comments( const std::wstring& );
        
    private:
        bool enable_;
        std::wstring formula_;
        std::wstring adduct_;
        std::wstring loss_;
        bool polarityPositive_;
        size_t chargeCount_;
        std::wstring comments_;
        
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int /* version */) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP(enable_);
            ar & BOOST_SERIALIZATION_NVP(formula_);
            ar & BOOST_SERIALIZATION_NVP(adduct_);
            ar & BOOST_SERIALIZATION_NVP(loss_);
            ar & BOOST_SERIALIZATION_NVP(polarityPositive_);
            ar & BOOST_SERIALIZATION_NVP(chargeCount_);
            ar & BOOST_SERIALIZATION_NVP(comments_);
        }
    };
    

    class ADCONTROLSSHARED_EXPORT MSRefSeries {
    public:
        ~MSRefSeries();
        MSRefSeries();
        MSRefSeries( const MSRefSeries& );

        bool useIsotopes() const;
        const std::wstring& repeat() const;
        const std::wstring& endGroup() const;
        const std::wstring& adduct() const;
        const std::wstring& loss() const;
        double lowMass();
        double highMass();
        bool polarityPositive() const;
        size_t chargeCount() const;
        const std::wstring& comments() const;

        void enable( bool );
        void repeat( const std::wstring& );
        void endGroup( const std::wstring& );
        void adduct( const std::wstring& );
        void loss( const std::wstring& );
        void lowMass( double );
        void highMass( double );
        void polarityPositive( bool );
        void chargeCount( size_t );
        void comments( const std::wstring& );

    private:
        bool enable_;
        std::wstring repeat_;
        std::wstring endGroup_;
        std::wstring adduct_;
        std::wstring loss_;
        double lowMass_;
        double highMass_;
        bool polarityPositive_;
        size_t chargeCount_;
        std::wstring comments_;
        
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int /* version*/) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP(enable_);
            ar & BOOST_SERIALIZATION_NVP(repeat_);
            ar & BOOST_SERIALIZATION_NVP(endGroup_);
            ar & BOOST_SERIALIZATION_NVP(adduct_);
            ar & BOOST_SERIALIZATION_NVP(loss_);
            ar & BOOST_SERIALIZATION_NVP(polarityPositive_);
            ar & BOOST_SERIALIZATION_NVP(chargeCount_);
            ar & BOOST_SERIALIZATION_NVP(lowMass_);
            ar & BOOST_SERIALIZATION_NVP(highMass_);
            ar & BOOST_SERIALIZATION_NVP(comments_);
        }
    };


    class ADCONTROLSSHARED_EXPORT MSReferenceDefns {
    public:
        ~MSReferenceDefns();
        MSReferenceDefns();
        MSReferenceDefns( const MSReferenceDefns& t );

        void addFormula( const MSRefFormula& );
        void addSeries( const MSRefSeries& );
        const std::vector< MSRefFormula >& formulae() const;
        const std::vector< MSRefSeries >& series() const;

    private:
        std::vector< MSRefFormula > refFormula_;
        std::vector< MSRefSeries > refSeries_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int /*version*/) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP(refFormula_);
            ar & BOOST_SERIALIZATION_NVP(refSeries_);
        }

    };

}


