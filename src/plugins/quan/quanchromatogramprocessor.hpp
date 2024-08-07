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
#include <boost/uuid/uuid.hpp>

namespace adcontrols {
    class ProcessMethod; class MSFinder;
    class PeakMethod;    class MSChromatogramMethod;
    namespace lockmass { class mslock; }
    class DataReader;
}
namespace adwidgets { class ProgressInterface; }

namespace adprocessor {
    namespace v3 { class MSChromatogramExtractor; }
}

namespace quan {

    class QuanSampleProcessor;
    class QuanDataWriter;
    class QuanTarget;

    class QuanChromatogramProcessor {

    public:
        QuanChromatogramProcessor( std::shared_ptr< const adcontrols::ProcessMethod > pm );

        //[[deprecated]] void process1st( int64_t pos, std::shared_ptr< adcontrols::MassSpectrum > ms, QuanSampleProcessor& sampleprocessor );

        // [[deprecated]] void doit( QuanSampleProcessor&, adcontrols::QuanSample&
        //                           , std::shared_ptr< QuanDataWriter >
        //                           , const std::string& reader_objtext
        //                           , std::shared_ptr< adwidgets::ProgressInterface > );

        bool operator()( QuanSampleProcessor&, adcontrols::QuanSample&, std::shared_ptr< QuanDataWriter >, std::shared_ptr< adwidgets::ProgressInterface > );

        enum { idFormula, idExactMass };
        typedef std::tuple< std::string, double > target_type;

        static bool findPeaks( adcontrols::PeakResult& res, const adcontrols::Chromatogram&, const adcontrols::PeakMethod& );
        static bool identify( adcontrols::PeakResult&, const adcontrols::QuanCompounds&, const adcontrols::Chromatogram& );

    private:
        static
        void extract_chromatograms_via_auto_target( QuanSampleProcessor& processor
                                                    , adcontrols::QuanSample& sample
                                                    , std::shared_ptr< QuanDataWriter > writer
                                                    , size_t idx
                                                    , const adcontrols::ProcessMethod& pm
                                                    , const adcontrols::QuanCompounds& cmpds
                                                    , adprocessor::v3::MSChromatogramExtractor& extractor
                                                    , std::shared_ptr< const adcontrols::DataReader > reader
                                                    , std::shared_ptr< adwidgets::ProgressInterface > progress );

        static
        void extract_chromatograms_via_mols( QuanSampleProcessor& processor
                                             , adcontrols::QuanSample& sample
                                             , std::shared_ptr< QuanDataWriter > writer
                                             , size_t idx
                                             , const adcontrols::ProcessMethod& pm
                                             , const adcontrols::QuanCompounds& cmpds
                                             , adprocessor::v3::MSChromatogramExtractor& extractor
                                             , std::shared_ptr< const adcontrols::DataReader > reader
                                             , std::shared_ptr< adwidgets::ProgressInterface > progress );

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

        void doCountingChromatogram( QuanSampleProcessor&, adcontrols::QuanSample&
                                     , std::shared_ptr< QuanDataWriter >
                                     , const std::string& reader_objtext
                                     , std::shared_ptr< adwidgets::ProgressInterface > );

        int debug_level_;
        bool save_on_datasource_;
        std::shared_ptr< adcontrols::ProcessMethod > procm_; // copy for average process
        std::array< std::unique_ptr< adcontrols::MSChromatogramMethod >, 2 > cXmethods_;
        std::map< size_t, QuanChromatograms::spectra_type > spectra_;
        std::shared_ptr< adcontrols::MSLockMethod > mslockm_;
        std::shared_ptr< adcontrols::lockmass::mslock > mslock_;
        std::vector< double > references_;
        std::vector< std::shared_ptr< QuanTarget > > targets_;
	};

}
