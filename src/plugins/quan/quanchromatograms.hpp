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

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace adcontrols {
    class CentroidMethod;
    class ChemicalFormula;
    class Chromatogram;
    class MassSpectrum;
    class MSLockMethod;
    class MSPeakInfo;
    class PeakResult;
    class Peaks;
    class Peak;
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
    class QuanChromatogram;
    class QuanCandidate;

    class QuanChromatograms {

        QuanChromatograms& operator = (const QuanChromatograms&) = delete;
        QuanChromatograms( const QuanChromatograms& ) = delete;
        
    public:
        ~QuanChromatograms();

        typedef std::tuple< std::string, double, int, double, double > computed_target_value;

        static inline const std::string& target_formula( computed_target_value& t ) { return std::get<0>(t) ; }
        static inline double target_exactMass( computed_target_value& t ) { return std::get<1>( t ); }
        static inline int target_charge( computed_target_value& t ) { return std::get<2>( t ); }
        static inline double target_matchedMass( computed_target_value& t ) { return std::get<3>( t ); }
        static inline double target_width( computed_target_value& t ) { return std::get<4>( t ); }
        
        QuanChromatograms( const std::string& formula
                           , const std::vector< computed_target_value >& target_values );

        void append_to_chromatogram( size_t pos, std::shared_ptr<const adcontrols::MassSpectrum> );
        void process_chromatograms( std::shared_ptr< const adcontrols::ProcessMethod > pm );
        bool identify( const adcontrols::QuanCompounds *, std::shared_ptr< const adcontrols::ProcessMethod > pm );

        enum { idProfile, idCentroid, idFiltered, idMSPeakInfo };

        typedef std::tuple< std::shared_ptr< adcontrols::MassSpectrum >
                            , std::shared_ptr< adcontrols::MassSpectrum >
                            , std::shared_ptr< adcontrols::MassSpectrum >
                            , std::shared_ptr< adcontrols::MSPeakInfo> > spectra_type;

        void refine_chromatograms( std::vector< QuanCandidate >& refined, std::function<spectra_type( uint32_t pos )> reader );
        
        enum process_phase { _1st, _2nd };

        typedef std::vector< std::shared_ptr< QuanChromatogram > >::const_iterator const_iterator;
        typedef std::vector< std::shared_ptr< QuanChromatogram > >::iterator iterator;        

        iterator begin() { return candidates_.begin(); }
        iterator end()   { return candidates_.end(); }
        
        const_iterator begin() const { return candidates_.begin(); }
        const_iterator end() const { return candidates_.end(); }

        void getPeaks( std::vector< std::pair< uint32_t, adcontrols::Peak * > >&, bool identified_only = true );
        uint32_t posFromPeak( uint32_t candidate_index, const adcontrols::Peak& ) const;

    private:
        std::string formula_;
        std::vector< computed_target_value > target_values_; // exact, matched, width
        
        enum { idFormula, idExactMass };
        typedef std::tuple< std::string, double > target_type;

        std::vector< std::shared_ptr< QuanChromatogram > > candidates_;

        adcontrols::QuanCompounds * compounds_;
        double tolerance_;
        std::vector< double > references_;
        double uptime_;
        std::shared_ptr< adcontrols::MSLockMethod > mslockm_;
        std::shared_ptr< adcontrols::lockmass > mslock_;
        bool identified_;
    };

}

