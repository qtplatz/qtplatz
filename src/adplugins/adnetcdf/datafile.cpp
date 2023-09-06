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

// #if __APPLE__ && (__GNUC_LIBSTD__ <= 4) && (__GNUC_LIBSTD_MINOR__ <= 2)
// #  define BOOST_NO_CXX11_RVALUE_REFERENCES
// #endif

#include "andichromatogram.hpp"
#if defined __GNUC__
# pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "datafile.hpp"
#include "ncfile.hpp"
// #include "attribute.hpp"
// #include "dimension.hpp"
// #include "variable.hpp"
#include <netcdf.h>
#include <adcontrols/countinghistogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/datainterpreterbroker.hpp>
#include <adcontrols/datapublisher.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adportable/textfile.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adlog/logger.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/tokenizer.hpp>
#include <boost/uuid/uuid.hpp>
#include <filesystem>

using namespace adnetcdf;

datafile::~datafile()
{
}

datafile::datafile() : accelVoltage_( 0 )
                     , length_( 0 )
                     , tDelay_( 0 )
{
}

void
datafile::accept( adcontrols::dataSubscriber& sub )
{
    ADDEBUG() << "================ " << __FUNCTION__ << " ==================";
    // subscribe acquired dataset <LCMSDataset>
    // No LC/GC data supported
    // sub.subscribe( *this );

    // subscribe processed dataset
    if ( processedDataset_ )
        sub.subscribe( *processedDataset_ );

    if ( auto db = sub.db() ) {
        if ( ! model_.empty() ) { // has scan law
            auto models = adcontrols::MassSpectrometer::installed_models();
            auto it = std::find_if( models.begin(), models.end(), [&]( const std::pair< boost::uuids::uuid, std::string >& t ){
                    return t.second == model_;
                });
            if ( it == models.end() )
                return; // can't find clsid

            static std::string misc_spectrometer = "9d7afc9f-f4c8-4a2d-a462-23aafde8d99f";

            adfs::stmt sql( *db );
            sql.prepare( "INSERT OR REPLACE INTO ScanLaw ("
                         " objuuid, objtext, acclVoltage, tDelay, spectrometer, clsidSpectrometer)"
                         " VALUES ( ?,?,?,?,?,? )" );
            sql.bind( 1 ) = boost::uuids::uuid{ 0 };           // master observer
            sql.bind( 2 ) = std::string( "master.observer" );  // signal observer text name
            sql.bind( 3 ) = this->accelVoltage_;
            sql.bind( 4 ) = this->tDelay_;
            sql.bind( 5 ) = misc_spectrometer; // ScanLaw.spectrometer = Spectrometer.id
            sql.bind( 6 ) = it->first;  // uuid_massspectrometer;

            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << "sqlite error";

            sql.prepare( "INSERT OR REPLACE INTO Spectrometer ( id, scanType, description, fLength ) VALUES ( ?,?,?,? )" );
            sql.bind( 1 ) = misc_spectrometer;  // ScanLaw.spectrometer = Spectrometer.id
            sql.bind( 2 ) = 0;
            sql.bind( 3 ) = std::string( "CSV Imported" );
            sql.bind( 4 ) = this->length_;

            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << "sqlite error";
        }
    }
}

bool
datafile::open( const std::wstring& filename, bool /* readonly */ )
{
    ADDEBUG() << "----------------- datafile::open(" << filename << ")";

    portfolio::Portfolio portfolio;
    portfolio.create_with_fullpath( filename );

    processedDataset_ = std::make_unique< adcontrols::ProcessedDataset >();
    processedDataset_->xml( portfolio.xml() );


    if ( auto file = adnetcdf::netcdf::open( boost::filesystem::path( filename ) ) ) {
        ADDEBUG() << file.path() << " open success.";
        ADDEBUG() << "file.kind: " << file.kind() << file.kind_extended();

        AndiChromatogram andi;
        if ( auto chro = andi.import( file ) ) {
            std::wstring name = std::filesystem::path( filename ).stem().wstring();
            auto folder = portfolio.addFolder( L"Chromatograms" );
            auto folium = folder.addFolium( name ).assign( chro, chro->dataClass() );


            return true;
        }
    }
    return false;
}


boost::any
datafile::fetch( const std::string& path, const std::string& dataType ) const
{
    ADDEBUG() << "================ fetch ==================";
    // return fetch( adportable::utf::to_wstring( path ), adportable::utf::to_wstring( dataType ) );
    return {};
}

boost::any
datafile::fetch( const std::wstring& path, const std::wstring& dataType ) const
{
    ADDEBUG() << "================ fetch ==================";
	return {};
}

size_t
datafile::getSpectrumCount( int /* fcn */ ) const
{
    ADDEBUG() << "================ " << __FUNCTION__ << " ==================";
    return 1;
}

bool
datafile::getSpectrum( int /* fcn */, size_t /* idx */, adcontrols::MassSpectrum&, uint32_t ) const
{
    ADDEBUG() << "================ " << __FUNCTION__ << " ==================";
    return true;
}

bool
datafile::getTIC( int /* fcn */, adcontrols::Chromatogram& ) const
{
    ADDEBUG() << "================ " << __FUNCTION__ << " ==================";
    return false;
}

size_t
datafile::getChromatogramCount() const
{
    ADDEBUG() << "================ " << __FUNCTION__ << " ==================";
    return 1;
}

size_t
datafile::getFunctionCount() const
{
    return 1;
}

size_t
datafile::posFromTime( double ) const
{
	return 0;
}

double
datafile::timeFromPos( size_t ) const
{
	return 0;
}
