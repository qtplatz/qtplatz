//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataprocessor.h"
#include "datafileimpl.h"
#include "constants.h"
#include <adcontrols/datafile.h>
#include <qtwrapper/qstring.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/ifile.h>

using namespace dataproc;

Dataprocessor::~Dataprocessor()
{
}

Dataprocessor::Dataprocessor()
{
}

bool
Dataprocessor::open(const QString &fileName )
{
    adcontrols::datafile * file = adcontrols::datafile::open( qtwrapper::wstring::copy( fileName ), true );
    if ( file ) {
        datafileimpl_.reset( new datafileimpl( file ) );
        file->accept( *datafileimpl_ );
        return true;
    }
    return false;
}

Core::IFile *
Dataprocessor::ifile()
{
    return static_cast<Core::IFile *>( datafileimpl_.get() );
}

adcontrols::datafile&
Dataprocessor::file()
{
    return datafileimpl_->file();
}

adcontrols::LCMSDataset *
Dataprocessor::getLCMSDataset()
{
    return datafileimpl_->getLCMSDataset();
}