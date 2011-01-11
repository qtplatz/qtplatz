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

#include "dataprocessorfactory.h"
#include "sessionmanager.h"
#include "dataprocessor.h"
#include "constants.h"
#include <coreplugin/ifilefactory.h>
#include <QStringList>
#include <adcontrols/datafile.h>
#include <qtwrapper/qstring.h>

using namespace dataproc::internal;

DataprocessorFactory::~DataprocessorFactory()
{
}

DataprocessorFactory::DataprocessorFactory( QObject * owner ) : Core::IFileFactory( owner )
                                                              , kind_( "Dataprocessor" )
{
    mimeTypes_ << Constants::C_DATAPROCESSOR_MIMETYPE;
}

// implementation for IFileFactory

QStringList 
DataprocessorFactory::mimeTypes() const
{
    return mimeTypes_;
}

QString 
DataprocessorFactory::kind() const
{
    return kind_;
}

Core::IFile * 
DataprocessorFactory::open( const QString& filename )
{
    boost::shared_ptr<Dataprocessor> processor( new Dataprocessor );
    if ( processor->open( filename ) ) {
        SessionManager::instance()->addDataprocessor( processor );
        return processor->ifile();
    }
    return 0;
}
