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
#include <adcontrols/datafile.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adfs/adfs.hpp>
#include <adfs/file.hpp>
#include <adlog/logger.hpp>

#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <QMessageBox>
#include <QFontMetrics>
#include <QApplication>

using namespace adprocessor;

dataprocessor::~dataprocessor()
{
}

dataprocessor::dataprocessor() : modified_( false )
                               , rawdata_( 0 )
{
}

void
dataprocessor::setModified( bool modified )
{
    modified_ = modified;
}

bool
dataprocessor::open( const QString& filename, QString& error_message )
{
    if ( auto file = std::unique_ptr< adcontrols::datafile >( adcontrols::datafile::open( filename.toStdWString(), false ) ) ) {
        file_ = std::move( file );
        file_->accept( *this );

        boost::filesystem::path path( filename.toStdWString() );
        
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

adcontrols::datafile *
dataprocessor::file()
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


///////////////////////////
bool
dataprocessor::subscribe( const adcontrols::LCMSDataset& data )
{
    rawdata_ = &data;
	return true;
}

void
dataprocessor::notify( adcontrols::dataSubscriber::idError, const wchar_t * )
{
}

