/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quansampleprocessor.hpp"
#include "quandatawriter.hpp"
#include "quandocument.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quansequence.hpp>
#include <adportable/debug.hpp>
#include <adfs/adfs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adfs/cpio.hpp>
#include <adlog/logger.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <boost/exception/all.hpp>
#include <algorithm>

using namespace quan;

QuanSampleProcessor::~QuanSampleProcessor()
{
}

QuanSampleProcessor::QuanSampleProcessor( const std::wstring& path
                                          , std::vector< adcontrols::QuanSample >& samples ) : raw_( 0 )
                                                                                             , path_( path )
                                                                                             , samples_( samples )
{
}

bool
QuanSampleProcessor::operator()( std::shared_ptr< QuanDataWriter > writer )
{
    open();

    for ( auto& sample : samples_ ) {

        switch ( sample.dataGeneration() ) {

        case adcontrols::QuanSample::GenerateChromatogram:
            break; // ignore for this version

        case adcontrols::QuanSample::GenerateSpectrum:
            if ( raw_ ) {
                //size_t nfcn = raw_->getFunctionCount();
                adcontrols::Chromatogram chro;
                if ( !raw_->getTIC( 0, chro ) )
                    break;
                size_t ndata = chro.size();
                
                auto range = std::make_pair( sample.scan_range_first(), sample.scan_range_second() );
                if ( range.first >= 0 ) {
                    range.second = std::min( uint32_t(ndata), range.second );
                    auto ms = std::make_shared< adcontrols::MassSpectrum >();
                    size_t pos = raw_->make_pos( range.first, 0 );
                    if ( raw_->getSpectrum( -1, pos, *ms ) ) {
                        for ( uint32_t i = range.first + 1; i < range.second; ++i ) {
                            size_t pos = raw_->make_pos( i, 0 );
                            adcontrols::MassSpectrum t;
                            if ( raw_->getSpectrum( -1, pos, t ) ) {
                                *ms += t;
                                ADDEBUG() << "getSpectrum( i=" << i << "," << pos << ")";
                            }
                        }
                    }
                    std::wstring id;
                    writer->write( *ms, sample.name(), id );
                }
            }
            break;

        case adcontrols::QuanSample::ASIS:
            do {
                if ( auto folder = portfolio_->findFolder( L"Spectra" ) ) {
                    if ( auto folium = folder.findFoliumByName( sample.name() ) ) {
                        if ( fetch( folium ) ) {
                            adcontrols::MassSpectrumPtr ms;
                            std::wstring id;
                            if ( portfolio::Folium::get< adcontrols::MassSpectrumPtr >( ms, folium ) )
                                writer->write( *ms, folium.name(), id );
                        }
                    }
                }
            } while ( 0 );
            break;
        }
    }
    QuanDocument::instance()->completed( this );
    return true;
}


void
QuanSampleProcessor::open()
{
    try {
        datafile_.reset( adcontrols::datafile::open( path_, true ) );
        if ( datafile_ )
            datafile_->accept( *this );
    }
    catch ( ... ) { ADERROR() << boost::current_exception_diagnostic_information(); }
}

bool
QuanSampleProcessor::subscribe( const adcontrols::LCMSDataset& d )
{
    raw_ = &d;
    return true;
}

bool
QuanSampleProcessor::subscribe( const adcontrols::ProcessedDataset& d )
{
    portfolio_ = std::make_shared< portfolio::Portfolio >( d.xml() );
    return true;
}

bool
QuanSampleProcessor::fetch( portfolio::Folium& folium )
{
    try {
        folium = datafile_->fetch( folium.id(), folium.dataClass() );
        portfolio::Folio attachs = folium.attachments();
        for ( auto att : attachs ) {
            if ( att.empty() )
                fetch( att ); // recursive call make sure for all blongings load up in memory.
        }
    }
    catch ( std::bad_cast& ) {}
    return true;
}
