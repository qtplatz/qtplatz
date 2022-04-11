/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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
#include "constants_fwd.hpp"
#include "targeting/isotope.hpp"
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace adcontrols {

    namespace targeting {
        class Candidate;
    }

    // this class is corresponding to targeting::Candidate class
    // due to historical reason, different json interface has been implemented.

    struct ADCONTROLSSHARED_EXPORT GenChromatogram;

    struct GenChromatogram {
        std::string formula;
        std::string display_name;
        double exact_mass;
        double exact_abundance;
        double mass;
        double time;
        int index;
        int proto;
        bool selected;
        std::vector< targeting::isotope > isotopes;
        GenChromatogram();
        GenChromatogram( const GenChromatogram& t );
        GenChromatogram( const targeting::Candidate&, bool selected = true );
    };

    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const GenChromatogram& );

    ADCONTROLSSHARED_EXPORT
    GenChromatogram tag_invoke( boost::json::value_to_tag< GenChromatogram >&, const boost::json::value& jv );
}
