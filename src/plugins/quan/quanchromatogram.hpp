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
}

namespace portfolio { class Portfolio; }

namespace quan {

    class QuanSampleProcessor;

    class QuanChromatogram {
        QuanChromatogram( const QuanChromatogram& ) = delete;
        QuanChromatogram& operator = ( const QuanChromatogram& ) = delete;

    public:
        QuanChromatogram( uint32_t fcn, uint32_t candidate_index, const std::string& formula, double exactMass, double matchedMass, const std::pair< double, double >& range );

        uint32_t candidate_index() const { return candidate_index_; }
        uint32_t fcn() const { return fcn_; }

        double matchedMass() const { return matchedMass_; }
        double exactMass() const { return exactMass_; }

        void setDataGuid( const std::wstring& dataGuid );
        const std::wstring& dataGuid() const;

        void setReferenceDataGuid( const std::wstring& dataGuid, uint32_t idx, uint32_t fcn );

        const std::vector< std::tuple< std::wstring, uint32_t, uint32_t > >& referenceDataGuids() { return dataGuids_; }

        uint32_t candidate_index() { return candidate_index_; }

        const std::string& formula() { return formula_; }

        void append( int64_t rowid, double time, double value );

        /** identify
         *  \brief identify peak that matches retention time for given formula listed in QuanCompounds
         */
        bool identify( const adcontrols::QuanCompounds&, const std::string& formula );

        bool is_identified() const;
        uint32_t identfied_peakid() const;

        const adcontrols::Peak * find_peak( uint32_t peakId ) const;
        std::vector< adcontrols::Peak * > find_peaks( const std::string& formula );

        uint32_t pos_from_peak( const adcontrols::Peak& ) const;
        std::vector< adcontrols::Peak * > peaks();
        
        inline bool operator < ( const QuanChromatogram& t ) const { return exactMass_ < t.exactMass_; }

        std::shared_ptr< adcontrols::Chromatogram > chromatogram() { return chromatogram_; }
        std::shared_ptr< adcontrols::PeakResult> peakResult()      { return peakinfo_; }

        const std::string& reader_objtext() const;
        void setReader_objtext( const std::string& );

        const std::pair< double, double >& msrange() const;

    private:
        std::shared_ptr< adcontrols::Chromatogram > chromatogram_;
        std::shared_ptr< adcontrols::PeakResult> peakinfo_;
        
        std::wstring dataGuid_;
        std::vector< std::tuple< std::wstring, uint32_t, uint32_t> > dataGuids_;  // reference spectra (guid,idx,fcn)
        std::string reader_objtext_;
        uint32_t fcn_;
        uint32_t candidate_index_;
        std::string formula_;
        double exactMass_;
        double matchedMass_;
        std::vector< int64_t > indices_; // adfs 'pos' - bin# map
        std::shared_ptr< adcontrols::MSPeakInfo > mspeaks_;
        std::pair< double, double > msrange_;
        uint32_t count_;
        
    private:
        uint32_t peakId_;
    };

} 

