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

#include <adcontrols/moltable.hpp>
#include <adcontrols/targeting.hpp>
#include <boost/optional.hpp>
#include <memory>

namespace adcontrols {
    class DataReader;
    class ProcessMethod;
    class MassSpectrum;
    namespace lockmass { class mslock; }
}

namespace adprocessor {

    class AutoTargetingCandidates {
        int proto_;
        adcontrols::moltable::value_type mol_;
        std::shared_ptr< adcontrols::MassSpectrum > refms_;
        std::shared_ptr< adcontrols::MassSpectrum > refms_processed_;
        std::vector< adcontrols::Targeting::Candidate > candidates_;
    public:
        AutoTargetingCandidates();
        AutoTargetingCandidates( const AutoTargetingCandidates& );
        AutoTargetingCandidates( int proto
                                 , const adcontrols::moltable::value_type& mol
                                 , std::shared_ptr< adcontrols::MassSpectrum > profile
                                 , std::shared_ptr< adcontrols::MassSpectrum > centroid );

        size_t size() const;
        boost::optional< adcontrols::Targeting::Candidate > operator []( size_t index ) const;

        void set_refms( std::shared_ptr< adcontrols::MassSpectrum > profile, std::shared_ptr< adcontrols::MassSpectrum > centroid );
        void set_candidates( const std::vector< adcontrols::Targeting::Candidate >& );
        void set_mol( const adcontrols::moltable::value_type& );

        const adcontrols::moltable::value_type& mol() const { return mol_; }
        std::shared_ptr< adcontrols::MassSpectrum > refms() const { return refms_; }
        std::shared_ptr< adcontrols::MassSpectrum > refms_processed() const { return refms_processed_; }
    };

}
