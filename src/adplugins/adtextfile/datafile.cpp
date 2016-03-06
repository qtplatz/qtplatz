// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <adcontrols/datafile.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/datainterpreterbroker.hpp>
#include <adcontrols/datapublisher.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adportable/textfile.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adlog/logger.hpp>
#include <QApplication>
#include <QMessageBox>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/tokenizer.hpp>

namespace adtextfile {
    
    struct text_parser {

        static bool find_spectrum( const std::wstring& filename, TXTSpectrum& txt, const Dialog& dlg ) {
            boost::filesystem::path path( filename );
            boost::filesystem::ifstream in( path );
            if ( in.fail() ) {
                QMessageBox::information(0, "Text file provider", QString("Cannot open fil: '%1'").arg( QString::fromStdWString( filename) ) );
                return false;
            }
            
            auto ms = std::make_shared< adcontrols::MassSpectrum >();
            for ( auto& model: adcontrols::MassSpectrometer::get_model_names() ) {
                if ( auto interpreter = adcontrols::DataInterpreterBroker::make_datainterpreter( adportable::utf::to_utf8(model )) ) {
                //if ( auto spectrometer = adcontrols::MassSpectrometer::find( model.c_str() ) ) {
                    if ( interpreter->compile_header( *ms, in ) ) {
                        return txt.load( filename, dlg );
                    }
                }
            }
            return false;
        }
    };

}

using namespace adtextfile;

datafile::~datafile()
{
}

datafile::datafile()
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
}

bool
datafile::open( const std::wstring& filename, bool /* readonly */ )
{
    portfolio::Portfolio portfolio;
    portfolio.create_with_fullpath( filename );

    Dialog dlg;
    dlg.setDataType( Dialog::data_spectrum );
    QStringList models;
    for ( auto& model: adcontrols::MassSpectrometer::get_model_names() )
        models << QString::fromStdWString( model );
    dlg.setDataInterpreterClsids( models );    
    
    boost::filesystem::path path( filename );
    boost::filesystem::ifstream in( path );
    if ( in.fail() ) {
        QMessageBox::information(0, "Text file provider", QString("Cannot open fil: '%1'").arg( QString::fromStdWString( filename) ) );
        return false;
    }
    typedef boost::char_separator<char> separator;
    typedef boost::tokenizer< separator > tokenizer;
    separator sep( ", \t", "", boost::drop_empty_tokens );
    std::string line;
    size_t nlines = 3;
    while ( nlines-- && adportable::textfile::getline( in, line ) ) {
        tokenizer tokens( line, sep );
        QStringList list;
        for ( tokenizer::iterator it = tokens.begin(); it != tokens.end(); ++it )
            list << QString::fromStdString( *it );
        dlg.appendLine( list );
    }

    QApplication::changeOverrideCursor( Qt::ArrowCursor );
    dlg.show();
    
    if ( dlg.exec() ) {
        
        QApplication::changeOverrideCursor( Qt::WaitCursor );
        
        if ( dlg.dataType() == Dialog::data_chromatogram ) {
            adtextfile::TXTChromatogram txt;
            portfolio::Portfolio portfolio;
            if ( txt.load( filename ) && prepare_portfolio( txt, filename, portfolio ) ) {
                processedDataset_.reset( new adcontrols::ProcessedDataset );
                processedDataset_->xml( portfolio.xml() );
                return true;                
            }
            
        } else if ( dlg.dataType() == Dialog::data_spectrum ) {
            TXTSpectrum txt;
            if ( txt.load( filename, dlg ) && prepare_portfolio( txt, filename, portfolio ) ) {
                processedDataset_.reset( new adcontrols::ProcessedDataset );
                processedDataset_->xml( portfolio.xml() );
                return true;                
            }
        }
    }
    return false;
}

boost::any
datafile::fetch( const std::wstring& path, const std::wstring& dataType ) const
{
    (void)dataType;

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

	return 0;
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
    boost::filesystem::path path( filename );
    
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
    boost::filesystem::path path( filename );
    
    int idx = 0;
    for ( auto it: txt.chromatograms_ ) {
        std::wstring name( (boost::wformat( L"%1%(%2%)" ) % path.stem().wstring() % idx++).str() );
        portfolio::Folium folium = chromatograms.addFolium( name );
        folium.setAttribute( L"dataType", adcontrols::Chromatogram::dataClass() );
        chro_[ folium.id() ] = it;
    }

    return true;
}

