/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quanconnection.hpp"
#include "quanplotdata.hpp"
#include "quanquery.hpp"
#include <adfs/cpio.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/quansample.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <dataproc/dataprocconstants.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <QMessageBox>

using namespace quan;

QuanConnection::~QuanConnection()
{
}

QuanConnection::QuanConnection()
{
}

bool
QuanConnection::connect( const std::wstring& database )
{
    if ( ( fs_ = std::make_shared< adfs::filesystem >() ) ) { // 
        if ( fs_->mount( database.c_str() ) ) {
            fs_->db().register_error_handler( [=](const char * msg){ QMessageBox::warning(0, "SQLite SQL Error", msg); });
            filename_ = database;
            return true;
        }
    }
    return false;
}

std::shared_ptr<QuanQuery>
QuanConnection::query()
{
    if ( fs_ )
        return std::make_shared<QuanQuery>( fs_->db() );
    return 0;
}

adfs::sqlite&
QuanConnection::db()
{
    if ( fs_ )
        return fs_->db();
    static adfs::sqlite dummy;
    return dummy;
}

adfs::file
QuanConnection::select_file( const std::wstring& dataGuid, const wchar_t * path )
{
    if ( auto folder = fs_->findFolder( path ) ) // L"/Processed/Spectra" | L"/Processed/Chromatograms
        return folder.selectFile( dataGuid );
    return adfs::file();
}

QuanPlotData *
QuanConnection::fetch( const std::wstring& dataGuid )
{
    if ( cache_.find( dataGuid ) != cache_.end() )
        return cache_[ dataGuid ].get();

    adfs::file file;
    if ( ( file = select_file( dataGuid, L"/Processed/Spectra" ) )
         || ( file = select_file( dataGuid, L"/Processed/Chromatograms" ) ) ) {

        auto d = std::make_shared< QuanPlotData >();

        if ( file.dataClass() == adcontrols::MassSpectrum::dataClass() ) {
            d->profile = std::make_shared< adcontrols::MassSpectrum >();
            try {
                if ( ! file.fetch( *d->profile ) )
                    return 0;
            } catch ( std::exception& ex ) {
                ADERROR() << boost::diagnostic_information( ex );
                return 0;
            }

        } else if ( file.dataClass() == adcontrols::Chromatogram::dataClass() ) {
            d->chromatogram = std::make_shared< adcontrols::Chromatogram >();
            try {
                if ( ! file.fetch( *d->chromatogram ) )
                    return 0;
            } catch ( std::exception& ex ) {
                ADERROR() << boost::diagnostic_information( ex );
                return 0;
            }
        }

        auto atts = file.attachments();
        for ( auto& att : atts ) {
            auto name = att.attribute( L"name" );

            if ( att.dataClass() == adcontrols::MassSpectrum::dataClass() ) {
                if ( att.attribute( L"name" ) == dataproc::Constants::F_CENTROID_SPECTRUM ) {

                    d->centroid = std::make_shared< adcontrols::MassSpectrum >();
                    att.fetch( *d->centroid );

                } else if ( att.attribute( L"name" ) == dataproc::Constants::F_DFT_FILTERD ) {

                    d->filterd = std::make_shared< adcontrols::MassSpectrum >();
                    att.fetch( *d->filterd );

                }
            } else if ( att.dataClass() == adcontrols::MSPeakInfo::dataClass() ) {

                d->pkinfo = std::make_shared< adcontrols::MSPeakInfo >();
                att.fetch( *d->pkinfo );

            } else if ( att.dataClass() == adcontrols::PeakResult::dataClass() ) {

                d->pkResult = std::make_shared< adcontrols::PeakResult >();
                att.fetch( *d->pkResult );

            } else if ( att.dataClass() == adcontrols::QuanSample::dataClass() ) {

                d->sample = std::make_shared< adcontrols::QuanSample >();
                att.fetch( *d->sample );

            } else if ( att.dataClass() == adcontrols::ProcessMethod::dataClass() ) {

                d->procmethod = std::make_shared< adcontrols::ProcessMethod >();
                att.fetch( *d->procmethod );                

            }
        }
        cache_[ dataGuid ] = d;
        return cache_[ dataGuid ].get();
    }
    return 0;
}

const adcontrols::ProcessMethod *
QuanConnection::processMethod()
{
    if ( !procmethod_ )
        readMethods();
    return procmethod_.get();
}

const adcontrols::QuanSequence *
QuanConnection::quanSequence()
{
    if ( !sequence_ )
        readMethods();
    return sequence_.get();
}

bool
QuanConnection::readMethods()
{
    sequence_ = std::make_shared< adcontrols::QuanSequence >();
    procmethod_ = std::make_shared< adcontrols::ProcessMethod >();

    if ( auto folder = fs_->findFolder( L"/Processed/Quan" ) ) {
 
        auto files = folder.files();
        // larger rowid first (decend order)
        std::sort( files.begin(), files.end(), [] ( const adfs::file& a, const adfs::file& b ){ return a.rowid() > b.rowid(); } );

        do {
            auto it = std::find_if( files.begin(), files.end()
                                    , [] ( const adfs::file& t ){ return t.dataClass() == adcontrols::ProcessMethod::dataClass(); } );
            if ( it != files.end() )
                it->fetch( *procmethod_ );
        } while ( false );

        do {
            auto it = std::find_if( files.begin(), files.end()
                                    , [] ( const adfs::file& t ){ return t.dataClass() == adcontrols::QuanSequence::dataClass(); } );
            if ( it != files.end() )
                it->fetch( *sequence_ );
        } while ( false );
    }
    return true;
}
