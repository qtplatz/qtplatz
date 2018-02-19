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
    namespace lockmass { class mslock; }
}
namespace adwidgets { class Progress; }

namespace quan {

    class QuanSampleProcessor;
    class QuanDataWriter;
    class QuanTarget;

    class QuanChromatogramProcessor {
        std::shared_ptr< const adcontrols::ProcessMethod > procm_;

    public:
        QuanChromatogramProcessor( std::shared_ptr< const adcontrols::ProcessMethod > pm );

        void process1st( size_t pos, std::shared_ptr< adcontrols::MassSpectrum > ms, QuanSampleProcessor& sampleprocessor );

        void doit( QuanSampleProcessor&, adcontrols::QuanSample&
                   , std::shared_ptr< QuanDataWriter >
                   , const std::string& reader_objtext
                   , std::shared_ptr< adwidgets::Progress > );

        enum { idFormula, idExactMass };
        typedef std::tuple< std::string, double > target_type;        

    private:
        bool doMSLock( adcontrols::MassSpectrum& profile );
        void correct_baseline( adcontrols::MassSpectrum& profile );

        void find_parallel_chromatograms( std::vector< std::shared_ptr< QuanChromatograms > >&
                                          , const std::vector< std::shared_ptr< QuanTarget > >& targets
                                          , const std::string& reader_objtext
                                          , double mspeak_width = 0.002
                                          , double tolerance = 0.010 );

        static std::wstring make_title( const wchar_t * dataSource, const std::string& formula, int fcn, double error, const wchar_t * trailer = L"" );
        static std::wstring make_title( const wchar_t * dataSource, const QuanCandidate&, const wchar_t * trailer = L"" );        

        static bool doCentroid( const adcontrols::MassSpectrum& profile
                                , const adcontrols::ProcessMethod& pm
                                , std::shared_ptr< adcontrols::MSPeakInfo >& pkInfo
                                , std::shared_ptr< adcontrols::MassSpectrum >& centroid
                                , std::shared_ptr< adcontrols::MassSpectrum >& filtered );
        
        void save_candidate_chromatograms( std::shared_ptr< QuanDataWriter > writer
                                           , const wchar_t * dataSource
                                           , std::shared_ptr< const QuanChromatograms >
                                           , const wchar_t * title_trailer = L"" );
        
        std::wstring save_spectrum( std::shared_ptr< QuanDataWriter > writer
                                    , const QuanCandidate&
                                    , const std::wstring& title );
        
        std::map< size_t, QuanChromatograms::spectra_type > spectra_;
        std::shared_ptr< adcontrols::MSLockMethod > mslockm_;
        std::shared_ptr< adcontrols::lockmass::mslock > mslock_;
        std::vector< double > references_;
        std::vector< std::shared_ptr< QuanTarget > > targets_;
	};
    
}

