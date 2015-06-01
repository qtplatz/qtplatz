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

    class QuanChromatogram {
        QuanChromatogram( const QuanChromatogram& ) = delete;
        QuanChromatogram& operator = ( const QuanChromatogram& ) = delete;

    public:
        uint32_t candidate_index() const { return candidate_index_; }
        uint32_t fcn() const { return fcn_; }
        double matchedMass() const { return matchedMass_; }
        double exactMass() const { return exactMass_; }
        
        uint32_t fcn_;
        uint32_t candidate_index_;
        
        std::shared_ptr< adcontrols::Chromatogram > chromatogram_;
        std::shared_ptr< adcontrols::PeakResult> peakinfo_;
        std::wstring dataGuid_;
        
        std::string formula_;
        double exactMass_;
        double matchedMass_;
        
        std::vector< uint32_t > indecies_;
        std::shared_ptr< adcontrols::QuanResponse > resp_;
        std::shared_ptr< adcontrols::MSPeakInfo > mspeaks_;
        std::pair< double, double > msrange_;

        QuanChromatogram( uint32_t fcn, uint32_t candidate_index, const std::string& formula, double exactMass, double matchedMass, const std::pair< double, double >& range );
        
        void append( uint32_t pos, double time, double value );

        bool identify( const adcontrols::QuanCompounds& );
        bool is_identified() const;
        uint32_t identfied_peakid() const;
        const adcontrols::Peak * find_peak( uint32_t peakId ) const;
        uint32_t pos_from_peak( const adcontrols::Peak& ) const;
        std::vector< adcontrols::Peak * > peaks( bool identified );
        
        inline bool operator < ( const QuanChromatogram& t ) const { return exactMass_ < t.exactMass_; }
    private:
        uint32_t peakId_;
    };

} 

