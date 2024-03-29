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
#include "quantarget.hpp"

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
    namespace lockmass { class mslock; }
}
namespace portfolio { class Portfolio; }

namespace quan {

    class QuanSampleProcessor;
    class QuanChromatogram;
    class QuanCandidate;
    struct peak_score_type;

    class QuanChromatograms {

        QuanChromatograms& operator = (const QuanChromatograms&) = delete;
        QuanChromatograms( const QuanChromatograms& ) = delete;
        
    public:
        ~QuanChromatograms();
        
        QuanChromatograms( const std::string& formula /* master formula -> developped to many by possible adducts|charge */
                           , const std::vector< QuanTarget::target_value >& target_values
                           , const std::string& reader_objtext );

        QuanChromatograms( const std::string& formula, const QuanCandidate&, double mass_width );

        void append_to_chromatogram( int64_t pos, std::shared_ptr<const adcontrols::MassSpectrum>, const std::string& reader_objtext );

        void process_chromatograms( std::shared_ptr< const adcontrols::ProcessMethod > pm );

        bool identify( const adcontrols::QuanCompounds *, std::shared_ptr< const adcontrols::ProcessMethod > pm );

        struct spectra_type {
            std::shared_ptr< adcontrols::MassSpectrum > profile;
            std::shared_ptr< adcontrols::MassSpectrum > centroid;
            std::shared_ptr< adcontrols::MassSpectrum > filtered;
            std::shared_ptr< adcontrols::MSPeakInfo> mspkinfo;
            
            spectra_type() {
            }
            
            spectra_type( std::shared_ptr< adcontrols::MassSpectrum > _profile
                          , std::shared_ptr< adcontrols::MassSpectrum > _centroid
                          , std::shared_ptr< adcontrols::MassSpectrum > _filtered
                          , std::shared_ptr< adcontrols::MSPeakInfo> _mspkinfo )
                : profile( _profile ), centroid( _centroid ), filtered( _filtered ), mspkinfo( _mspkinfo ) {
            }
            
            spectra_type( const spectra_type& t )
                : profile( t.profile ), centroid( t.centroid ), filtered( t.filtered ), mspkinfo( t.mspkinfo ) {
            }
            
        };

        void refine_chromatograms( const std::string& reader_objtext, std::vector< QuanCandidate >& refined, std::function<spectra_type( uint32_t pos )> reader );

        void finalize( std::function<spectra_type( uint32_t pos )> reader );
        
        typedef std::vector< std::shared_ptr< QuanChromatogram > >::const_iterator const_iterator;
        typedef std::vector< std::shared_ptr< QuanChromatogram > >::iterator iterator;        

        iterator begin() { return qchro_.begin(); }
        iterator end()   { return qchro_.end(); }
        
        const_iterator begin() const { return qchro_.begin(); }
        const_iterator end() const { return qchro_.end(); }
        std::shared_ptr< QuanCandidate > quanCandidate() { return candidate_; }
        void refactor();

    private:
        std::string reader_objtext_;
        std::string formula_;
        std::vector< QuanTarget::target_value > target_values_; // exact, matched, width
        std::vector< std::shared_ptr< QuanChromatogram > > qchro_;
        std::shared_ptr< QuanCandidate > candidate_;
        std::vector< double > references_;
        double tolerance_;
        double uptime_;
        std::shared_ptr< adcontrols::MSLockMethod > mslockm_;
        std::shared_ptr< adcontrols::lockmass::mslock > mslock_;
        bool identified_;

        void refine_identified_chromatograms( std::function<spectra_type( uint32_t pos )> reader, std::vector< peak_score_type >& );
        void refine_unidentified_chromatograms( std::function<spectra_type( uint32_t pos )> reader, std::vector< peak_score_type >& );
    };

}

