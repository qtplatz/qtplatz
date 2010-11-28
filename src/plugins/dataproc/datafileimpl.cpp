// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "datafileimpl.h"
#include <adcontrols/lcmsdataset.h>
#include <adcontrols/processeddataset.h>
#include <adcontrols/massspectrum.h>
#include <qtwrapper/qstring.h>
#include <xmlwrapper/qtxml.h>

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

/*
QString
datafileimpl::fileFilter() const
{
return QString( "*.data" );
}

QString
datafileimpl::fileExtension() const
{
return QString( ".data" );
}
*/

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

    using namespace xmlwrapper::qtxml;

    XMLDocument dom;
    bool res = dom.loadXML( xml );
    XMLElement elm = dom.documentElement();
    xmlstring str = elm.nodeName();
    if ( res )
        long x = 1;
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
