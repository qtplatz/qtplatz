/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "import.hpp"
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adinterface/signalobserver.hpp>
#include <adutils/acquiredconf.hpp>
#include <adutils/acquireddata.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/cpio.hpp>
#include <boost/filesystem.hpp>
#include <thread>
#include <chrono>

using namespace batchproc;

import::import()
{
}

import::import( int row
                , const std::wstring& source_file
                , const std::wstring& destination_file
                , std::function< bool(int, int, int) > progress )
    : rowId_( row )
    , source_file_( source_file )
    , destination_file_( destination_file )
    , progress_( progress )
    , datafile_(0)
    , accessor_(0)
    , fs_( new adfs::filesystem )
    , objId_( 1 )
{
    datafile_ = adcontrols::datafile::open( source_file, true );

    if ( destination_file.empty() ) {

        boost::filesystem::path src( source_file );

        boost::filesystem::path dir( adportable::profile::user_data_dir< char >() );
        dir /= "data";
		dir /= src.parent_path().leaf();

        if ( !boost::filesystem::exists( dir ) )
            boost::filesystem::create_directories( dir );

		boost::filesystem::path path( dir / src.filename() );
        path.replace_extension( L".adfs" );

		destination_file_ = path.generic_wstring();

        if ( ! fs_->create( destination_file_.c_str() ) ) {
            adcontrols::datafile::close( datafile_ );
            datafile_ = 0;
        }
        adutils::AcquiredConf::create_table( fs_->db() );
        adutils::AcquiredData::create_table( fs_->db() );
        
        adutils::AcquiredConf::insert( fs_->db()
                                       , objId_ 
                                       , 0   // pobjid
                                       , L"batchproc::import"
                                       , uint64_t( signalobserver::eTRACE_SPECTRA )
                                       , uint64_t( signalobserver::eMassSpectrometer )
                                       , L"MS.PROFILE"
                                       , L"Profile mass spectrum"
                                       , L"m/z"
                                       , L"intensity"
                                       , 4
                                       , 0 );

        adutils::AcquiredConf::insert( fs_->db()
                                       , objId_ + 1
                                       , 0   // pobjid
                                       , L"batchproc::import"
                                       , uint64_t( signalobserver::eTRACE_TRACE )
                                       , uint64_t( signalobserver::eMassSpectrometer )
                                       , L"MS.TIC"
                                       , L"TIC"
                                       , L"time"
                                       , L"intensity"
                                       , 4
                                       , 0 );
    }
}

import::~import()
{
    if ( datafile_ )
        adcontrols::datafile::close( datafile_ );
}

bool
import::operator()()
{
    if ( datafile_ ) {

        datafile_->accept( *this );
        if ( tic_.empty() )
            return false;

        const adcontrols::Chromatogram& chro = *tic_[0];
        size_t nSpectra = chro.size();
        if ( nSpectra > 0 ) {
            adfs::detail::cpio obuf;
            if ( adfs::cpio< adcontrols::Chromatogram >::serialize( chro, obuf ) ) {
                uint32_t events = 0;
                int64_t time = 0; // us
                uint32_t fcn = 0;
                int32_t pos = 0;
                adutils::AcquiredData::insert( fs_->db(), objId_ + 1, time, pos, fcn, events, obuf.get(), obuf.size() );
            }
        }

        for ( size_t i = 0; i < nSpectra; ++i ) {

            if ( progress_( rowId_, i, chro.size() ) ) {
                progress_( rowId_, 0, 0 ); // canceled
                return false;
            }

            adcontrols::MassSpectrum ms;
            if ( accessor_->getSpectrum( 0, i, ms ) ) {
                const adcontrols::MSProperty& prop = ms.getMSProperty();

                adfs::detail::cpio obuf;
                if ( adfs::cpio< adcontrols::MassSpectrum >::serialize( ms, obuf ) ) {
                    uint64_t time = static_cast<uint64_t>( adcontrols::metric::scale_to_micro( prop.timeSinceInjection() ) );
                    uint32_t events = 0;
                    adutils::AcquiredData::insert( fs_->db(), objId_, time, i, 0, events, obuf.get(), obuf.size() );
                }

            }

        }

        progress_( rowId_, nSpectra, nSpectra ); // completed
        return true;
    }
    progress_( rowId_, 0, 0 ); // 
    return false;
}

bool
import::subscribe( const adcontrols::LCMSDataset& data )
{
    accessor_ = &data;
    size_t nfcn = data.getFunctionCount();
    for ( size_t i = 0; i < nfcn; ++i ) {
        auto c = std::make_shared< adcontrols::Chromatogram >();
        if ( data.getTIC( i, *c ) )
            tic_.push_back( c );
    }
    return true;
}
