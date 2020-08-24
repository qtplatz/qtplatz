/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "admtcontrols_global.hpp"
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <vector>
#include <string>

namespace boost { namespace serialization { class access; } }

namespace admtcontrols {
    class ADMTCONTROLSSHARED_EXPORT OrbitProtocol;
    class ADMTCONTROLSSHARED_EXPORT DelayMethod;

    class DelayMethod {
    public:
        double delay; // seconds
        double width; // seconds
        bool enable;

        DelayMethod( double _d = 0, double _w = 0, bool enable = true );
        DelayMethod( const DelayMethod& t );

    private:
        friend class boost::serialization::access;
        template< class Archive >
            void serialize( Archive& ar, const unsigned int );
    };


    template<typename T> class OrbitProtocol_archive;

    //--
    class OrbitProtocol {

    public:
        enum eItem { MCP_V, IONIZATION_V, NAVERAGE, GAIN, NINTVAL, FIL_A };

        double lower_mass;    // lower mass
        double upper_mass;    // upper mass
        double avgr_delay;    // seconds
        double avgr_duration; // seconds
        DelayMethod pulser;   // A
        DelayMethod inject;   // B
        DelayMethod exit;     // D
        DelayMethod exit2;    // <-- added at V9
        std::vector< DelayMethod > gate;  // C
        DelayMethod external_adc_delay;
        std::string& description();
        const std::string& description() const;
        std::string& formulae();
        const std::string& formulae() const;
        std::vector< std::pair< eItem, int32_t > >& additionals();
        const std::vector< std::pair< eItem, int32_t > >& additionals() const;
        uint32_t& nlaps();
        uint32_t nlaps() const;
        uint32_t& reference();
        uint32_t reference() const;

        uint32_t& replicates();
        uint32_t replicates() const;

    private:
        std::vector< std::pair< eItem, int32_t > > additionals_;
        uint32_t nlaps_;        // number of laps
        uint32_t reference_;    // lock mass reference (bit position indicate which formula in formulae
        std::string formulae_;  // formula list, separate with ';'
        std::string description_;
        // serializer v8
        uint32_t replicates_;

    public:
        OrbitProtocol();
        OrbitProtocol( const OrbitProtocol& t );

    private:
        friend class boost::serialization::access;
        template< class Archive >
            void serialize( Archive& ar, const unsigned int version );
        friend class OrbitProtocol_archive < OrbitProtocol > ;
        friend class OrbitProtocol_archive < const OrbitProtocol > ;
    };

#if defined _MSC_VER
    /* ADMTCONTROLSSHARED_TEMPLATE_EXPORT */ template class ADMTCONTROLSSHARED_EXPORT std::vector< DelayMethod >;
    /* ADMTCONTROLSSHARED_TEMPLATE_EXPORT */ template class ADMTCONTROLSSHARED_EXPORT std::vector< OrbitProtocol >;
#endif

}

BOOST_CLASS_VERSION( admtcontrols::OrbitProtocol, 9 )
BOOST_CLASS_VERSION( admtcontrols::DelayMethod, 1 )
