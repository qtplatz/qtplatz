//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataprocessorfactory.h"
#include "dataprocplugin.h"
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

DataprocessorFactory::DataprocessorFactory( DataprocPlugin * owner ) : Core::IFileFactory( owner )
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
        DataprocPlugin::instance()->getDataprocessors().push_back( processor );
        return processor->ifile();
    }
    return 0;
}
