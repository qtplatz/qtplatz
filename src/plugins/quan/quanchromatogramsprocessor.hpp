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

#include "quanchromatograms.hpp"

namespace adcontrols {
    class ProcessMethod; class MSFinder;
}
namespace adwidgets { class Progress; }

namespace quan {

    class QuanSampleProcessor;
    class QuanDataWriter;

    class QuanChromatogramProcessor {
        std::shared_ptr< const adcontrols::ProcessMethod > procm_;

    public:
        QuanChromatogramProcessor( std::shared_ptr< const adcontrols::ProcessMethod > pm );

        void process1st( size_t pos, std::shared_ptr< adcontrols::MassSpectrum > ms, QuanSampleProcessor& sampleprocessor );

        void doit( QuanSampleProcessor&, adcontrols::QuanSample&
                   , std::shared_ptr< QuanDataWriter >, std::shared_ptr< adwidgets::Progress > );

    private:
        void process_chromatograms( QuanSampleProcessor& sampleprocessor, QuanChromatograms::process_phase phase );

        static std::wstring make_title( const wchar_t * dataSource, const std::string& formula, QuanChromatograms::process_phase phase );

        void save_candidate_chromatograms( std::shared_ptr< QuanDataWriter > writer
                                           , const wchar_t * dataSource
                                           , QuanChromatograms::process_phase phase
                                           , std::shared_ptr< adwidgets::Progress > progress );

        static bool doCentroid( const adcontrols::MassSpectrum& profile
                                , const adcontrols::ProcessMethod& pm
                                , std::shared_ptr< adcontrols::MSPeakInfo >& pkInfo
                                , std::shared_ptr< adcontrols::MassSpectrum >& centroid
                                , std::shared_ptr< adcontrols::MassSpectrum >& filtered );
        
        
        size_t collect_candidate_spectra( std::shared_ptr< adwidgets::Progress > progress );
        
        bool assign_mspeak( const adcontrols::MSFinder& find, QuanChromatogram& c );
        
        bool identify_cpeak( adcontrols::QuanResponse& resp
                             , const std::string& formula
                             , std::shared_ptr< adcontrols::Chromatogram > chro
                             , std::shared_ptr< adcontrols::PeakResult > pkResult );

        void save_candidate_spectra( std::shared_ptr< QuanDataWriter > writer
                                     , adcontrols::QuanSample& sample
                                     , std::shared_ptr< adwidgets::Progress > progress );

        
        std::shared_ptr< QuanChromatograms > chroms_;
        std::map< size_t, std::shared_ptr< adcontrols::MassSpectrum > > spectra_;
    };
    
}

