// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC
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

#if defined __GNUC__
# pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "datafile.hpp"
#include "dialog.hpp"
#include "txtspectrum.hpp"
#include "txtchromatogram.hpp"
#include "time_data_reader.hpp"
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
#include <adportable/csv_reader.hpp>
#include <adportable/debug.hpp>
#include <adportable/textfile.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adlog/logger.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <QApplication>
#include <QMessageBox>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/fstream.hpp>
//#include <boost/tokenizer.hpp>
#include <boost/uuid/uuid.hpp>
#include <filesystem>

using namespace adtextfile;

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
datafile::open( const std::filesystem::path& path, bool /* readonly */ )
{
    portfolio::Portfolio portfolio;
    portfolio.create_with_fullpath( path );

    Dialog dlg;

    QStringList models;
    for ( auto [uuid,model]: adcontrols::MassSpectrometer::installed_models() )
        models << QString::fromStdString( model );

    dlg.setDataInterpreterClsids( models );

    std::string adfsname;

    if ( time_data_reader::is_time_data( path.string(), adfsname ) ) {
        dlg.setDataType( Dialog::counting_time_data );
        double acclVoltage(0), tDelay(0), fLength(0);
        std::string spectrometer;
        if ( time_data_reader::readScanLaw( adfsname, acclVoltage, tDelay, fLength, spectrometer ) )
            dlg.setScanLaw( acclVoltage, tDelay, fLength, QString::fromStdString( spectrometer ) );

    } else {
        dlg.setDataType( Dialog::data_spectrum );
    }

    std::ifstream in( path );
    if ( in.fail() ) {
        QMessageBox::information(0, "Text file provider"
                                 , QString("Cannot open fil: '%1'").arg( QString::fromStdWString( path.wstring() ) ) );
        return false;
    }

    size_t nlines = 50;
    adportable::csv::list_string_type alist;
    adportable::csv::csv_reader reader;
    while ( nlines--  && reader.read( in, alist ) && in.good() ) {
        QStringList list;
        for ( const auto& value: alist )
            list << QString::fromStdString( std::get<1>(value) );
        dlg.appendLine( list );
    }

    QApplication::changeOverrideCursor( Qt::ArrowCursor );
    dlg.show();

    if ( dlg.exec() ) {

        //qtwrapper::waitCursor wait;
        QApplication::changeOverrideCursor( Qt::WaitCursor );

        if ( dlg.hasScanLaw() ) {
            accelVoltage_ = dlg.acceleratorVoltage();
            length_ = dlg.length();
            tDelay_ = dlg.tDelay();
            model_ = dlg.dataInterpreterClsid().toStdString();
        }

        if ( dlg.dataType() == Dialog::data_chromatogram ) {
            adtextfile::TXTChromatogram txt;
            portfolio::Portfolio portfolio;
            if ( txt.load( path.wstring() ) && prepare_portfolio( txt, path.wstring(), portfolio ) ) {
                processedDataset_.reset( new adcontrols::ProcessedDataset );
                processedDataset_->xml( portfolio.xml() );
                return true;
            }

        } else if ( dlg.dataType() == Dialog::data_spectrum ) {
            TXTSpectrum txt;
            if ( txt.load( path.wstring(), dlg ) && prepare_portfolio( txt, path.wstring(), portfolio ) ) {
                processedDataset_.reset( new adcontrols::ProcessedDataset );
                processedDataset_->xml( portfolio.xml() );
                return true;
            }
        } else if ( dlg.dataType() == Dialog::counting_time_data ) {
            time_data_reader reader;
            if ( reader.load( path.string()
                              , [&]( size_t numerator, size_t denominator ){
                                  ADDEBUG() << "Processing: " << path.string()
                                            << boost::format( "\t%.1f%%\r")
                                      % (double( numerator ) * 100 / double(denominator) );
                                  return true;
                              }) && prepare_portfolio( reader, path.wstring(), portfolio ) ) {
                processedDataset_.reset( new adcontrols::ProcessedDataset );
                processedDataset_->xml( portfolio.xml() );
                return true;
            }
        }
    }
    return false;
}


boost::any
datafile::fetch( const std::string& path, const std::string& dataType ) const
{
    return fetch( adportable::utf::to_wstring( path ), adportable::utf::to_wstring( dataType ) );
}

boost::any
datafile::fetch( const std::wstring& path, const std::wstring& dataType ) const
{
    do { // find from a spectrum tree
        auto it = data_.find( path );
        if ( it != data_.end() )
            return it->second;
    } while ( 0 );
    do { // find from a chromatogram tree
        auto it = chro_.find( path );
        if ( it != chro_.end() )
            return it->second;
    } while ( 0 );

	return {};
}

size_t
datafile::getSpectrumCount( int /* fcn */ ) const
{
    return 1;
}

bool
datafile::getSpectrum( int /* fcn */, size_t /* idx */, adcontrols::MassSpectrum&, uint32_t ) const
{
    return true;
}

bool
datafile::getTIC( int /* fcn */, adcontrols::Chromatogram& ) const
{
    return false;
}

size_t
datafile::getChromatogramCount() const
{
    return 0;
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

bool
datafile::prepare_portfolio( const TXTSpectrum& txt, const std::wstring& filename, portfolio::Portfolio& portfolio )
{
    portfolio::Folder spectra = portfolio.addFolder( L"Spectra" );
    std::filesystem::path path( filename );

    int idx = 0;
    for ( auto it: txt.spectra_ ) {
        std::wstring name( (boost::wformat( L"%1%(%2%)" ) % path.stem().wstring() % idx++).str() );
        portfolio::Folium folium = spectra.addFolium( name );
        folium.setAttribute( L"dataType", adcontrols::MassSpectrum::dataClass() );
        data_[ folium.id() ] = it;
    }

    if ( txt.spectra_.size() > 1 ) {
        do {
            auto it = txt.spectra_.begin();
            auto ptr = std::make_shared< adcontrols::MassSpectrum >( *it->get() );

            std::for_each( it + 1, txt.spectra_.end()
                           , [&ptr] ( std::shared_ptr< adcontrols::MassSpectrum > sub ) { *ptr << std::move( sub ); } );

            std::wstring name = path.stem().wstring();
            portfolio::Folium folium = spectra.addFolium( name );
            folium.setAttribute( L"dataType", adcontrols::MassSpectrum::dataClass() );
            data_[ folium.id() ] = ptr;

        } while(0);
    }

    return true;
}

bool
datafile::prepare_portfolio( const TXTChromatogram& txt, const std::wstring& filename, portfolio::Portfolio& portfolio )
{
    portfolio::Folder chromatograms = portfolio.addFolder( L"Chromatograms" );
    std::filesystem::path path( filename );

    int idx = 0;
    for ( auto it: txt.chromatograms_ ) {
        std::wstring name( (boost::wformat( L"%1%(%2%)" ) % path.stem().wstring() % idx++).str() );
        portfolio::Folium folium = chromatograms.addFolium( name );
        folium.setAttribute( L"dataType", adcontrols::Chromatogram::dataClass() );
        chro_[ folium.id() ] = it;
    }

    return true;
}

bool
datafile::prepare_portfolio( const time_data_reader& reader, const std::wstring& filename, portfolio::Portfolio& portfolio )
{
    portfolio::Folder spectra = portfolio.addFolder( L"Spectra" );
    std::filesystem::path path( filename );

    adcontrols::CountingHistogram hgrm;
    for ( auto& trig: reader.data() )
        hgrm << trig;

    const auto& front = *hgrm.begin();
    const auto& back  = *( hgrm.begin() + ( hgrm.size() - 1 ) );

    adcontrols::MSProperty prop;
    double delay      = front.first;
    uint32_t nDelay   = delay / hgrm.xIncrement();
    uint32_t nSamples = ( back.first - front.first ) / hgrm.xIncrement();

    adcontrols::SamplingInfo info( hgrm.xIncrement(), delay, nDelay, uint32_t( nSamples ), /* number of average */ 1, /*mode*/ 0 );
    prop.setSamplingInfo( info );

    auto sp = std::make_shared< adcontrols::MassSpectrum >();
    sp->setMSProperty( prop );
    sp->setCentroid( adcontrols::CentroidNative );
    sp->resize( hgrm.size() );
    size_t idx(0);
    for ( auto& pair: hgrm ) {
        sp->setTime( idx, pair.first );
        sp->setIntensity( idx, pair.second.size() );
        // ADDEBUG() << idx << ", " << pair.first << ", " << pair.second.size();
        ++idx;
    }

    std::wstring name( ( boost::wformat( L"%1%(%2%)" ) % path.stem().wstring() % idx++).str() );
    portfolio::Folium folium = spectra.addFolium( name );
    folium.setAttribute( L"dataType", adcontrols::MassSpectrum::dataClass() );

    data_[ folium.id() ] = sp;

    return true;
}
