/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adprocessor_global.hpp"
#include <adcontrols/moltable.hpp>
#include <boost/optional.hpp>
#include <memory>

namespace adcontrols {
    class DataReader;
    class ProcessMethod;
    class MassSpectrum;
    namespace lockmass { class mslock; }
}

namespace adprocessor {

    class AutoTargetingCandidates;

    class ADPROCESSORSHARED_EXPORT AutoTargeting;

    class AutoTargeting {
    public:
        // original implementation
        boost::optional< double > find( int proto
                                        , const adcontrols::moltable::value_type& mol
                                        , const adcontrols::ProcessMethod& pm
                                        , std::shared_ptr< const adcontrols::DataReader > reader
                                        , std::function< void( const adcontrols::lockmass::mslock& )> callback );

        // refactord implementation
        AutoTargetingCandidates doit( int proto
                                      , const adcontrols::moltable::value_type& mol
                                      , const adcontrols::ProcessMethod& pm
                                      , std::shared_ptr< const adcontrols::DataReader > reader
                                      , std::function< void( const adcontrols::lockmass::mslock& )> callback );
    };
}
