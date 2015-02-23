/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <tuple>

namespace adcontrols {
    class QuanSample;
    class QuanCompounds;
    class ProcessMethod;
    class MassSpectrum;
    class MSPeakInfo;
    class CentroidMethod;
    class MSLockMethod;
    class TargetingMethod;
    class ChemicalFormula;
    class Chromatogram;
    class lockmass;
}
namespace portfolio { class Portfolio; }

namespace quan {

    class QuanChromatograms {

        QuanChromatograms& operator = (const QuanChromatograms&) = delete;

    public:
        ~QuanChromatograms();
        QuanChromatograms( const std::shared_ptr< adcontrols::ProcessMethod> );
        QuanChromatograms( const QuanChromatograms& );

        bool mslock( adcontrols::MassSpectrum& ms );
        bool processIt( size_t pos, adcontrols::MassSpectrum& ms );
        void save( portfolio::Portfolio& portfolio );

    private:
        enum { idFormula, idMass, idChromatogram };
        typedef std::tuple< std::string, double, std::shared_ptr< adcontrols::Chromatogram > > target_t;

        std::vector< target_t > targets_; // order by mass

        const std::shared_ptr< adcontrols::ProcessMethod > pm_;
        adcontrols::QuanCompounds * compounds_;
        double tolerance_;
        std::vector< double > references_;
        double uptime_;
        std::shared_ptr< adcontrols::MSLockMethod > mslockm_;
        std::shared_ptr< adcontrols::lockmass > mslock_;
    };

}

