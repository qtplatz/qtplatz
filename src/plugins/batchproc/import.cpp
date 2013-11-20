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
#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif

#include "import.hpp"
#include "importdata.hpp"
#include "task.hpp"

#include <adcontrols/datafile.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/cpio.hpp>
#include <adinterface/signalobserver.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/serializer.hpp>
#include <adportable/float.hpp>
#include <adportable/bzip2.hpp>
#include <adutils/acquiredconf.hpp>
#include <adutils/acquireddata.hpp>

#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>

#include <thread>
#include <chrono>
#include <algorithm>

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
    , profileId_( 1 )
    , centroidId_( profileId_ + 1 )
    , ticId_( profileId_ + 2 )
{
    // open should be run in main thread with respect to CoInitialize (for Bruker CompassXtrace)
    datafile_ = adcontrols::datafile::open( source_file_, true );
}

import::~import()
{
    if ( datafile_ ) {
        adcontrols::datafile::close( datafile_ );
        fs_->close();
        fs_.reset();

        boost::filesystem::path path( destination_file_ );
        path.replace_extension( L".adfs" );
        try {
            if ( boost::filesystem::exists( path ) )
                boost::filesystem::remove( path );
        } catch ( std::exception& ex ) {
            adportable::debug(__FILE__, __LINE__) << "remove file " << path.string() << " caught an exception: " << ex.what();
        }
        try {
            boost::filesystem::rename( destination_file_, path );
        } catch ( std::exception& ex ) {
            adportable::debug(__FILE__, __LINE__) << "rename file " << destination_file_ << " to " << path.string() << " caught an exception: " << ex.what();
        }
    }
}

bool
import::operator()()
{
    if ( datafile_ && open_destination() ) {

        datafile_->accept( *this );
        if ( tic_.empty() )
            return false;

        const adcontrols::Chromatogram& chro = *tic_[0];
        size_t nSpectra = chro.size();
        if ( nSpectra > 0 ) {
            std::string archived;
            if ( adfs::cpio< adcontrols::Chromatogram >::serialize( chro, archived ) ) {
                std::string compressed;
                adportable::bzip2::compress( compressed, archived.data(), archived.size() );

                uint32_t events = 0;
                int64_t time = 0; // us
                uint32_t fcn = 0;
                int32_t pos = 0;
                adutils::AcquiredData::insert( fs_->db(), ticId_, time, pos, fcn, events, compressed.data(), compressed.size() );
            }
        }
        if ( accessor_->hasProcessedSpectrum( 0, 0 ) )
            import_processed_spectra( 0, nSpectra );
        import_profile_spectra( 0, nSpectra );
        task::instance()->remove( *this );

        return true;
    }
    return false;
}

bool
import::open_destination()    
{
    if ( datafile_ ) {

        if ( destination_file_.empty() ) {
            
            boost::filesystem::path src( source_file_ );

            boost::filesystem::path dir( adportable::profile::user_data_dir< char >() );
            dir /= "data";
            dir /= src.parent_path().leaf();

            if ( !boost::filesystem::exists( dir ) )
                boost::filesystem::create_directories( dir );

            boost::filesystem::path path( dir / src.filename() );
            path.replace_extension( L".adfs~" );

            destination_file_ = path.generic_wstring();
		}

        if ( ! fs_->create( destination_file_.c_str() ) ) {
            adcontrols::datafile::close( datafile_ );
            return false;
        }

        adutils::AcquiredConf::create_table( fs_->db() );
        adutils::AcquiredData::create_table( fs_->db() );
        
        adutils::AcquiredConf::insert( fs_->db()
                                       , profileId_ 
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
                                       , centroidId_
                                       , 0   // pobjid
                                       , L"batchproc::import"
                                       , uint64_t( signalobserver::eTRACE_SPECTRA )
                                       , uint64_t( signalobserver::eMassSpectrometer )
                                       , L"MS.CENTROID"
                                       , L"Profile mass spectrum"
                                       , L"m/z"
                                       , L"intensity"
                                       , 4
                                       , 0 );

        adutils::AcquiredConf::insert( fs_->db()
                                       , ticId_
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
        return true;
    }
    return false;
}

bool
import::import_processed_spectra( uint64_t fcn, size_t nSpectra )
{
	uint32_t objId = accessor_->findObjId( L"MS.CENTROID" );

    for ( size_t i = 0; i < nSpectra; ++i ) {

        if ( progress_( rowId_, i, nSpectra ) ) {
            progress_( rowId_, 0, 0 ); // canceled
            return false;
        }
        
        adcontrols::MassSpectrum ms;
        if ( accessor_->getSpectrum( fcn, i, ms, objId ) ) {

            if ( ! ms.isCentroid() )
                continue;

            std::string ar;
            if ( adfs::cpio< adcontrols::MassSpectrum >::serialize( ms, ar ) ) {
                //std::string compressed;
                //adportable::bzip2::compress( compressed, ar.data(), ar.size() );

                uint64_t time = static_cast<uint64_t>( adcontrols::metric::scale_to_micro( ms.getMSProperty().timeSinceInjection() ) );
                uint32_t events = 0;
                adutils::AcquiredData::insert( fs_->db(), centroidId_, time, i, fcn, events, ar.data(), ar.size() );
                
                adportable::debug(__FILE__, __LINE__) << "import spectrum size " << ar.size();
            }
        }  
    }
    progress_( rowId_, nSpectra, nSpectra ); // completed
    return true;
}

bool
import::import_profile_spectra( uint64_t fcn, size_t nSpectra )
{
    import_continuum_massarray meta;
    
    for ( size_t i = 0; i < nSpectra; ++i ) {

        if ( progress_( rowId_, i, nSpectra ) ) {
            progress_( rowId_, 0, 0 ); // canceled
            return false;
        }
        
        adcontrols::MassSpectrum ms;
        if ( accessor_->getSpectrum( fcn, i, ms ) ) {
            bool keep_masses = true;
            
            if ( i == 0 ) {
                keep_masses = false;
            } else {
                if ( ms.size() != meta.masses_.size() )
                    keep_masses = false;
                
                const double * org = meta.masses_.data();
                const double * ptr = ms.getMassArray();
                
                for ( size_t k = 0; k < ms.size(); ++k ) {
                    if ( ! adportable::compare<double>::is_equal( *org++, *ptr++ ) ) {
                        keep_masses = false;
                        break;
                    }
                }
            }
            
            import_profile profile;
            setup_continuum_spectrum( profile, ms );
            
            if ( !keep_masses )
                setup_continuum_massarray( meta, ms );
            
            std::string obuf;
            boost::iostreams::back_insert_device< std::string > inserter( obuf );
            boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > astrm( inserter );

            std::string archive_data;
            std::string archive_meta;
            //std::string compressed_data;
            std::string compressed_meta;

            do {
                if ( adportable::serializer< import_profile >::serialize( profile, archive_data ) ) {
                    //adportable::bzip2::compress( compressed_data, archive_data.data(), archive_data.size() );
                }
            } while(0);

            if ( !keep_masses ) {
                if ( adportable::serializer< import_continuum_massarray >::serialize( meta, archive_meta ) ) {
                    adportable::bzip2::compress( compressed_meta, archive_meta.data(), archive_meta.size() );
                }
            }

            uint64_t time = static_cast<uint64_t>( adcontrols::metric::scale_to_micro( ms.getMSProperty().timeSinceInjection() ) );
            uint32_t events = 0;
            adutils::AcquiredData::insert( fs_->db(), profileId_, time, i, 0, events
                                           , archive_data.data(), archive_data.size()
                                           , compressed_meta.data(), compressed_meta.size() );
            
            adportable::debug(__FILE__, __LINE__)
                << "import spectrum size " << archive_data.size() << " mass array size: " << archive_meta.size();
        }
    }
    progress_( rowId_, nSpectra, nSpectra ); // completed
    return true;
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

void
import::setup_continuum_massarray( import_continuum_massarray& d, const adcontrols::MassSpectrum& ms )
{
    const double * p = ms.getMassArray();
	d.masses_.resize( ms.size() );
	std::copy( p, p + ms.size(), d.masses_.begin() );
}

void
import::setup_continuum_spectrum( import_profile& d, const adcontrols::MassSpectrum& ms )
{
    const double * p = ms.getIntensityArray();
    d.prop_ = ms.getMSProperty();
    d.polarity_ = ms.polarity();
	d.intensities_.resize( ms.size() );
	std::copy( p, p + ms.size(), d.intensities_.begin() );
}
