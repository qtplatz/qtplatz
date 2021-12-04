/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include <adcontrols/lockmass.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <boost/optional.hpp>

namespace adcontrols {
    class MSChromatogramMethod;
    class MSLockMethod;
    class MassSpectrum;
    class ProcessMethod;
}

namespace adprocessor {

    struct msLocker {
        std::vector< adcontrols::moltable::value_type > refs_;
        adcontrols::MSLockMethod lockm_;
        msLocker( const adcontrols::MSChromatogramMethod& cm, const adcontrols::ProcessMethod& pm );
        boost::optional< adcontrols::lockmass::mslock >  operator()( const adcontrols::MassSpectrum& centroid );
    };
}
