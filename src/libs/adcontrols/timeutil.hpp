// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include "metricprefix.hpp"
#include <utility>
#include <boost/serialization/nvp.hpp>
#include <cmath>

namespace adcontrols {

    struct ADCONTROLSSHARED_EXPORT seconds_t { 
        seconds_t( double t = 0 ) : seconds(t) {}
        seconds_t( const seconds_t& );
        seconds_t( const struct minutes_t& );
        double seconds; 
        operator double () const { return seconds; }
    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int /* version */) {
            ar & BOOST_SERIALIZATION_NVP( seconds );
        }
    };

    struct ADCONTROLSSHARED_EXPORT minutes_t {
        minutes_t( double t = 0 ) : minutes(t) {}
        minutes_t( const minutes_t& );
        minutes_t( const seconds_t& );
        double minutes; 
        operator double () const { return minutes; }
    };

    struct ADCONTROLSSHARED_EXPORT timeutil {
        static minutes_t toMinutes( const seconds_t& );
        static seconds_t toSeconds( const minutes_t& );
        static std::pair<double, double> toMinutes( const std::pair<seconds_t, seconds_t>& pair );
    };

}


