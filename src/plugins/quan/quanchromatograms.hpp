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
#include <array>

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

    class QuanChromatogram {
    public:
        std::string formula_;
        double exactMass_;
        double matchedMass_;
        std::vector< std::wstring > dataGuids_;  // dataGuids for reference spectra (eseentially, single item)
        std::vector< uint32_t > indecies_;
        std::array< std::wstring, 2 > dataGuid_;  // dataGuid for phase chromatograms
        std::array< std::shared_ptr< adcontrols::Chromatogram >, 2 > cmgrs_;
        std::array< std::shared_ptr< adcontrols::PeakResult>, 2 > pkres_;
        std::shared_ptr< adcontrols::QuanResponse > resp_;
        std::shared_ptr< adcontrols::MassSpectrum > profile_;
        std::shared_ptr< adcontrols::MassSpectrum > filtered_;
        std::shared_ptr< adcontrols::MassSpectrum > centroid_;
        std::shared_ptr< adcontrols::MSPeakInfo > mspeaks_;
        std::pair< size_t, size_t > idxfcn_;
        std::pair< double, double > mswidth_;
        QuanChromatogram() : exactMass_( 0 ), matchedMass_( 0 )
            {}
        QuanChromatogram( const QuanChromatogram& t ) : formula_( t.formula_ )
                                                      , exactMass_( t.exactMass_ )
                                                      , matchedMass_( t.matchedMass_ )
                                                      , dataGuid_( t.dataGuid_ )
                                                      , dataGuids_( t.dataGuids_ )
                                                      , indecies_( t.indecies_ )
                                                      , cmgrs_( t.cmgrs_ )
                                                      , pkres_( t.pkres_ )
                                                      , resp_( t.resp_ )
                                                      , profile_( t.profile_ )
                                                      , centroid_( t.centroid_ )
                                                      , mspeaks_( t.mspeaks_ )
                                                      , idxfcn_( t.idxfcn_ )
                                                      , mswidth_( t.mswidth_ ) {
        }

        QuanChromatogram( const std::string& formula
                          , double exactMass
                          , std::pair< size_t, size_t > idxfcn ) : formula_( formula )
                                                                 , exactMass_( exactMass )
                                                                 , idxfcn_( idxfcn )
            {}
        
        inline bool operator < ( const QuanChromatogram& t ) const { return exactMass_ < t.exactMass_; }
    };

    class QuanChromatograms {

        QuanChromatograms& operator = (const QuanChromatograms&) = delete;

    public:
        ~QuanChromatograms();
        QuanChromatograms( std::shared_ptr< const adcontrols::ProcessMethod> );
        QuanChromatograms( const QuanChromatograms& );


        enum process_phase { _1st, _2nd };

        bool doMSLock( adcontrols::MassSpectrum& ms );
        bool process1st( size_t pos, adcontrols::MassSpectrum& ms, QuanSampleProcessor& );
        bool process2nd( size_t pos, const adcontrols::MassSpectrum& ms );
        void process_chromatograms( QuanSampleProcessor&, process_phase );
        bool lockmass_enabled() const;
        bool apply_lockmass( size_t pos, adcontrols::MassSpectrum& ms, QuanSampleProcessor& );
        
        typedef std::vector< QuanChromatogram >::const_iterator const_iterator;
        typedef std::vector< QuanChromatogram >::iterator iterator;        

        iterator begin() { return candidates_.begin(); }
        iterator end()   { return candidates_.end(); }
        
        const_iterator begin() const { return candidates_.begin(); }
        const_iterator end() const { return candidates_.end(); }        
        
    private:
        enum { idFormula, idExactMass };
        typedef std::tuple< std::string, double > target_type;

        std::vector< target_type > targets_;
        std::vector< QuanChromatogram > candidates_; // order by mass

        std::shared_ptr< const adcontrols::ProcessMethod > pm_;
        adcontrols::QuanCompounds * compounds_;
        double tolerance_;
        std::vector< double > references_;
        double uptime_;
        std::shared_ptr< adcontrols::MSLockMethod > mslockm_;
        std::shared_ptr< adcontrols::lockmass > mslock_;
    };

}

