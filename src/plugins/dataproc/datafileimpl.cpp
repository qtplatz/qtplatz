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

#include "datafileimpl.h"
#include <adcontrols/lcmsdataset.h>
#include <adcontrols/processeddataset.h>
#include <adcontrols/massspectrum.h>
#include <qtwrapper/qstring.h>
#include <portfolio/portfolio.h>

using namespace dataproc;

datafileimpl::~datafileimpl()
{
    adcontrols::datafile::close( file_ );
}

datafileimpl::datafileimpl( adcontrols::datafile * file
                           , QObject *parent) : Core::IFile(parent)
                                              , modified_(false)
                                              , file_(file)
                                              , accessor_(0) 
                                              , filename_( qtwrapper::qstring::copy( file->filename() ) ) 
{
}

void
datafileimpl::setModified( bool val )
{
    if ( modified_ == val )
        return;
    modified_ = val;
    emit changed();
}

bool
datafileimpl::isModified() const
{
    return modified_;
}

QString
datafileimpl::mimeType() const
{
    return mimeType_;
}

bool
datafileimpl::save( const QString& filename )
{
    Q_UNUSED(filename);
    return false;
}

QString
datafileimpl::fileName() const
{
    return filename_;
}

QString
datafileimpl::defaultPath() const
{
    return "C:/Data";
}

QString
datafileimpl::suggestedFileName() const
{
    return QString();
}

bool
datafileimpl::isReadOnly() const
{
    if ( file_ && file_->readonly() )
        return true;
    return false;
}

bool
datafileimpl::isSaveAsAllowed() const
{
    return true;
}

void
datafileimpl::modified( ReloadBehavior* behavior )
{
    Q_UNUSED(behavior);
}

///////////////////////////
void
datafileimpl::subscribe( adcontrols::LCMSDataset& data )
{
    accessor_ = &data;
    size_t nfcn = data.getFunctionCount();
    for ( size_t i = 0; i < nfcn; ++i ) {
        adcontrols::Chromatogram c;
        if ( data.getTIC( i, c ) )
            ticVec_.push_back( c );
    }
}

void
datafileimpl::subscribe( adcontrols::ProcessedDataset& processed )
{
    std::wstring xml = processed.xml();
}


adcontrols::LCMSDataset *
datafileimpl::getLCMSDataset()
{
    return accessor_;
}

adcontrols::datafile&
datafileimpl::file()
{
    return *file_;
}
