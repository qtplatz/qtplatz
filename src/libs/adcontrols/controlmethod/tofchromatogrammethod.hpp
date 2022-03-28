// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "../adcontrols_global.h"
#include "constants_fwd.hpp"
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <string>
#include <vector>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT TofChromatogramMethod {
    public:
        ~TofChromatogramMethod();
        TofChromatogramMethod();
        TofChromatogramMethod( const TofChromatogramMethod& );

        bool enable() const;
        void setEnable( bool );

        /** Chemical formula for generate trace
         */
        const std::string& formula() const;
        void setFormula( const std::string& );

        /** Mono isotopic mass for generate trace
         */
        double mass() const;
        void setMass( double );

        double massWindow() const;
        void setMassWindow( double );

        /** If specified as non zero (> 1ns) value, create chromatogram with tof specified
         */
        double time() const;
        void setTime( double seconds );

        const double timeWindow() const;
        void setTimeWindow( double seconds );

        xic::eIntensityAlgorishm intensityAlgorithm() const;
        void setIntensityAlgorithm( xic::eIntensityAlgorishm );

        void setProtocol( int );
        int protocol() const;

        void setId( int );
        int id() const;

    private:
        class impl;
        impl * impl_;
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
        friend ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const TofChromatogramMethod& );
        friend ADCONTROLSSHARED_EXPORT TofChromatogramMethod
        tag_invoke( boost::json::value_to_tag< TofChromatogramMethod >&, const boost::json::value& jv );
    };

    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const TofChromatogramMethod& );

    ADCONTROLSSHARED_EXPORT
    TofChromatogramMethod tag_invoke( boost::json::value_to_tag< TofChromatogramMethod >&, const boost::json::value& jv );
}
