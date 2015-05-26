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
    class CentroidMethod;
    class ChemicalFormula;
    class Chromatogram;
    class MassSpectrum;
    class MSLockMethod;
    class MSPeakInfo;
    class PeakResult;
    class ProcessMethod;
    class QuanCompounds;
    class QuanResponse;
    class QuanSample;
    class TargetingMethod;
    class lockmass;
}
namespace portfolio { class Portfolio; }

namespace quan {

    class QuanSampleProcessor;

    class QuanChromatograms {

        QuanChromatograms& operator = (const QuanChromatograms&) = delete;

    public:
        ~QuanChromatograms();
        QuanChromatograms( std::shared_ptr< const adcontrols::ProcessMethod> );
        QuanChromatograms( const QuanChromatograms& );

        bool doMSLock( adcontrols::MassSpectrum& ms );
        bool process1st( size_t pos, adcontrols::MassSpectrum& ms, QuanSampleProcessor& );
        bool process2nd( size_t pos, const adcontrols::MassSpectrum& ms );
        void commit( QuanSampleProcessor& ); //portfolio::Portfolio& portfolio );
        bool lockmass_enabled() const;

        enum { idFormula, idMass, idChromatogram, idPeakResult, idQuanResponse, idSpectrum, idCentroid, idMSPeakInfo, idIdxFcn, idMatchedMass, idMSWidth };
        typedef std::tuple< std::string
                            , double
                            , std::shared_ptr< adcontrols::Chromatogram >
                            , std::shared_ptr< adcontrols::PeakResult>
                            , std::shared_ptr< adcontrols::QuanResponse >
                            , std::shared_ptr< adcontrols::MassSpectrum > // profile
                            , std::shared_ptr< adcontrols::MassSpectrum > // centroid
                            , std::shared_ptr< adcontrols::MSPeakInfo >
                            , std::pair< size_t, size_t > // idx, fcn
                            , double // matched mass
                            , std::pair<double, double > // ms left, right
                            > target_t;

        typedef std::vector< target_t >::const_iterator const_iterator;
        typedef std::vector< target_t >::iterator iterator;        

        iterator begin() { return targets_.begin(); }
        iterator end()   { return targets_.end(); }
        
        const_iterator begin() const { return targets_.begin(); }
        const_iterator end() const { return targets_.end(); }        
        
    private:

        std::vector< target_t > targets_; // order by mass

        std::shared_ptr< const adcontrols::ProcessMethod > pm_;
        adcontrols::QuanCompounds * compounds_;
        double tolerance_;
        std::vector< double > references_;
        double uptime_;
        std::shared_ptr< adcontrols::MSLockMethod > mslockm_;
        std::shared_ptr< adcontrols::lockmass > mslock_;
    };

}

