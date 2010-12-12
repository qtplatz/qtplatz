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
#include <portfolio/portfolio.h>
#include <adcontrols/lcmsdataset.h>
#include <adcontrols/processeddataset.h>


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
        file->accept( *this );
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

///////////////////////////
void
Dataprocessor::subscribe( adcontrols::LCMSDataset& data )
{
    size_t nfcn = data.getFunctionCount();
    for ( size_t i = 0; i < nfcn; ++i ) {
        adcontrols::Chromatogram c;
        if ( data.getTIC( i, c ) )
            ; // ticVec_.push_back( c );
    }
}

void
Dataprocessor::subscribe( adcontrols::ProcessedDataset& processed )
{
    std::wstring xml = processed.xml();
    portfolio_.reset( new portfolio::Portfolio( xml ) );
}
