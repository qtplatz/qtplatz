// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "ifileimpl.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <qtwrapper/qstring.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <boost/filesystem/path.hpp>

using namespace dataproc;

IFileImpl::~IFileImpl()
{
    adcontrols::datafile::close( file_ );
}


IFileImpl::IFileImpl( adcontrols::datafile * file
                     , Dataprocessor& dprocessor
                     , QObject *parent) : Core::IFile(parent)
                                        , modified_(false)
                                        , file_(file)
                                        , accessor_(0)
                                        , dprocessor_( dprocessor ) 
{
    if ( file_ )
        filename_ = QString( qtwrapper::qstring::copy( file_->filename() ) );
}

void
IFileImpl::setModified( bool val )
{
    if ( modified_ == val )
        return;
    modified_ = val;
    emit changed();
}

bool
IFileImpl::isModified() const
{
    return modified_;
}

QString
IFileImpl::mimeType() const
{
    return mimeType_;
}

bool
IFileImpl::save( const QString& filename )
{
    portfolio::Portfolio portfolio = dprocessor_.getPortfolio();

    boost::filesystem::path p( qtwrapper::wstring::copy( filename ) );
    p.replace_extension( L".adfs" );

    if ( boost::filesystem::path( qtwrapper::wstring::copy( filename_ ) ) == p ) { // same file?
        // save
        return this->file().saveContents( L"/Processed", portfolio );

    } else {
        // saveFileAs -- has to create new file
        boost::scoped_ptr< adcontrols::datafile > file( adcontrols::datafile::create( p.c_str() ) );
        return file && file->saveContents( L"/Processed", portfolio, this->file() );

    }
    return true;
}

QString
IFileImpl::fileName() const
{
    return filename_;
}

QString
IFileImpl::defaultPath() const
{
    return "C:/Data";
}

QString
IFileImpl::suggestedFileName() const
{
    return QString();
}

bool
IFileImpl::isReadOnly() const
{
    if ( file_ && file_->readonly() )
        return true;
    return false;
}

bool
IFileImpl::isSaveAsAllowed() const
{
    return true;
}

void
IFileImpl::modified( ReloadBehavior* behavior )
{
    Q_UNUSED(behavior);
}

///////////////////////////
bool
IFileImpl::subscribe( const adcontrols::LCMSDataset& data )
{
    accessor_ = &data;
    size_t nfcn = data.getFunctionCount();
    for ( size_t i = 0; i < nfcn; ++i ) {
        adcontrols::Chromatogram c;
        if ( data.getTIC( i, c ) )
            ticVec_.push_back( c );
    }
    return true;
}

bool
IFileImpl::subscribe( const adcontrols::ProcessedDataset& processed )
{
    std::wstring xml = processed.xml();
    return true;
}


const adcontrols::LCMSDataset *
IFileImpl::getLCMSDataset()
{
    return accessor_;
}

adcontrols::datafile&
IFileImpl::file()
{
    return *file_;
}

