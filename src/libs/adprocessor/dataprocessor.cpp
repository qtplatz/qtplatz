// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "dataprocessor.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adfs/adfs.hpp>
#include <adfs/file.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportfolio/portfolio.hpp>

#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace adprocessor;

dataprocessor::~dataprocessor()
{
}

dataprocessor::dataprocessor() : modified_( false )
                               , rawdata_( 0 )
                               , portfolio_( std::make_unique< portfolio::Portfolio >() )
{
}

void
dataprocessor::setModified( bool modified )
{
    modified_ = modified;
}

bool
dataprocessor::open( const std::wstring& filename, std::wstring& error_message )
{
    if ( auto file = std::unique_ptr< adcontrols::datafile >( adcontrols::datafile::open( filename, false ) ) ) {

        file_ = std::move( file );
        file_->accept( *this );

        boost::filesystem::path path( filename );
        
        auto fs = std::make_unique< adfs::filesystem >();
        if ( fs->mount( path ) ) {
            fs_ = std::move( fs );
        } else {
            path.replace_extension( ".adfs" );
            if ( ! boost::filesystem::exists( path ) ) {
                if ( fs->create( path ) )
                    fs_ = std::move( fs );
            }
        }
        
        return true;
    }
    return false;
}

const std::wstring&
dataprocessor::filename() const
{
    return file_->filename();
}

void
dataprocessor::setFile( std::unique_ptr< adcontrols::datafile >&& file )
{
    file_ = std::move( file );

    if ( file_ ) {
        file_->accept( *this );
    }
    
}

adcontrols::datafile *
dataprocessor::file()
{
    return file_.get();
}

const adcontrols::datafile *
dataprocessor::file() const
{
    return file_.get();
}

const adcontrols::LCMSDataset *
dataprocessor::rawdata()
{
    return rawdata_;
}

std::shared_ptr< adfs::sqlite >
dataprocessor::db() const
{
    if ( fs_ )
        return fs_->_ptr();
    else
        return nullptr;
}

const portfolio::Portfolio&
dataprocessor::portfolio() const
{
    return *portfolio_;
}

portfolio::Portfolio&
dataprocessor::portfolio()
{
    return *portfolio_;
}

///////////////////////////
bool
dataprocessor::subscribe( const adcontrols::LCMSDataset& data )
{
    rawdata_ = &data;
	return true;
}

bool
dataprocessor::subscribe( const adcontrols::ProcessedDataset& processed )
{
    std::string xml = processed.xml();
    portfolio_ = std::make_unique< portfolio::Portfolio >( xml );

    return true;
}

void
dataprocessor::notify( adcontrols::dataSubscriber::idError, const wchar_t * )
{
}

std::shared_ptr< adcontrols::MassSpectrum >
dataprocessor::readSpectrumFromTimeCount()
{
    std::shared_ptr< adcontrols::MassSpectrum > ms;
    
    adfs::stmt sql( *this->db() );
    sql.prepare( "SELECT objuuid from AcquiredConf WHERE objtext like 'histogram.timecount.1.%' LIMIT 1" );
    if ( sql.step() == adfs::sqlite_row ) {

        auto objuuid = sql.get_column_value< boost::uuids::uuid >( 0 );
        
        if ( auto raw = this->rawdata() ) {
            if ( auto reader = raw->dataReader( objuuid ) ) {
                auto it = reader->begin();
                ms = reader->readSpectrum( it );
            }
        }
    }
    
    boost::uuids::uuid clsidSpectrometer{ 0 };
    double acclVoltage(0), tDelay(0), fLength(0);
    sql.prepare( "SELECT acclVoltage,tDelay,fLength,clsidSpectrometer FROM ScanLaw,Spectrometer WHERE id=clsidSpectrometer LIMIT 1" );
    if ( sql.step() == adfs::sqlite_row ) {
        acclVoltage = sql.get_column_value< double >( 0 );
        tDelay      = sql.get_column_value< double >( 1 );
        fLength     = sql.get_column_value< double >( 2 );
        clsidSpectrometer = sql.get_column_value< boost::uuids::uuid >( 3 );
    }
    
    if ( auto spectrometer = adcontrols::MassSpectrometerBroker::make_massspectrometer( clsidSpectrometer ) ) {
        
        spectrometer->setScanLaw( acclVoltage, tDelay, fLength );
        auto scanlaw = spectrometer->scanLaw();
        
        {
            // lead control method
            auto idstr = boost::lexical_cast< std::string >( adcontrols::ControlMethod::Method::clsid() );
            sql.prepare( "SELECT data FROM MetaData WHERE clsid = ?" );
            sql.bind( 1 ) = idstr;
            while( sql.step() == adfs::sqlite_row ) {
                adcontrols::ControlMethod::Method m;
                auto blob = sql.get_column_value< adfs::blob >( 0 );
                boost::iostreams::basic_array_source< char > device( reinterpret_cast< const char * >( blob.data() ), size_t( blob.size() ) );
                boost::iostreams::stream< boost::iostreams::basic_array_source< char > > strm( device );
                if ( adcontrols::ControlMethod::Method::restore( strm, m ) )
                    spectrometer->setMethod( m );
            }
        }
        
        std::shared_ptr< adcontrols::MassSpectrum > hist = std::make_shared< adcontrols::MassSpectrum >();
        hist->clone( *ms );
        hist->setCentroid( adcontrols::CentroidNative );
        
        std::vector< double > t, y, m;
        double ptime(0);
        sql.prepare( "SELECT ROUND(peak_time, 9) AS time, COUNT(*), protocol FROM peak,trigger WHERE id=idTrigger GROUP BY time ORDER BY time" );
        while ( sql.step() == adfs::sqlite_row ) {

            double time = sql.get_column_value< double >( 0 ); // time
            int proto = sql.get_column_value< uint64_t >( 2 );                    
            
            if ( (time - ptime) > 1.2e-9 ) {
                if ( ptime > 1.0e-9 ) {
                    // add count(0) to the end of last cluster
                    t.emplace_back( ptime + 1.0e-9 );
                    y.emplace_back( 0 ); // count
                    m.emplace_back( scanlaw->getMass( ( ptime + 1.0e-9 ), spectrometer->mode( proto ) ) );
                }
                // add count(0) to the begining of next cluster
                t.emplace_back( time - 1.0e-9 );
                y.emplace_back( 0 ); // count
                m.emplace_back( scanlaw->getMass( ( time - 1.0e-9 ), spectrometer->mode( proto ) ) );
            }
            t.emplace_back( time );
            y.emplace_back( sql.get_column_value< uint64_t >( 1 ) ); // count
            m.emplace_back( scanlaw->getMass( time, spectrometer->mode( proto ) ) );
            ptime = time;
        }
        // close cluster if exists
        // t.emplace_back( ptime + 1.0e-9 );
        // y.emplace_back( 0 ); // count
        // m.emplace_back( scanlaw->getMass( ( ptime + 1.0e-9 ), spectrometer->mode( proto ) ) );
        
        hist->setMassArray( std::move( m ) );
        hist->setTimeArray( std::move( t ) );
        hist->setIntensityArray( std::move( y ) );

        return hist;
    }
}

