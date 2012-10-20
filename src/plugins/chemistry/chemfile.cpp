/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "chemfile.hpp"
// #include "dataprocessor.hpp"
// #include <adcontrols/lcmsdataset.hpp>
// #include <adcontrols/processeddataset.hpp>
// #include <adcontrols/massspectrum.hpp>
#include <qtwrapper/qstring.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

using namespace Chemistry::Internal;

ChemFile::~ChemFile()
{
    // adcontrols::datafile::close( file_ );
}


//ChemFile::ChemFile( adcontrols::datafile * file
//                    , Dataprocessor& dprocessor
//                    , QObject *parent) : Core::IFile( parent )
//                                       , modified_( false )
//                                       , file_( file )
//                                       , accessor_( 0 )
//                                       , dprocessor_( dprocessor ) 
ChemFile::ChemFile( QObject * parent ) : Core::IFile( parent )
	                                   , modified_( false )
{
//    if ( file_ )
//        filename_ = QString( qtwrapper::qstring::copy( file_->filename() ) );
}

void
ChemFile::setModified( bool val )
{
    if ( modified_ == val )
        return;
    modified_ = val;
    emit changed();
}

bool
ChemFile::isModified() const
{
    return modified_;
}

QString
ChemFile::mimeType() const
{
    return mimeType_;
}

bool
ChemFile::save( const QString& filename )
{
/*
    portfolio::Portfolio portfolio = dprocessor_.getPortfolio();

    boost::filesystem::path p( qtwrapper::wstring::copy( filename ) );
    p.replace_extension( L".adfs" );

    do {
        boost::filesystem::path xmlfile( filename.toStdString() );
        xmlfile.replace_extension( ".xml" );
        boost::filesystem::remove( xmlfile );
        pugi::xml_document dom;
        dom.load( pugi::as_utf8( portfolio.xml() ).c_str() );
        dom.save_file( xmlfile.string().c_str() );
    } while(0);

    if ( boost::filesystem::path( qtwrapper::wstring::copy( filename_ ) ) == p ) { // same file?
        // save
        return this->file().saveContents( L"/Processed", portfolio );

    } else {
        // saveFileAs -- has to create new file
        boost::filesystem::remove( boost::filesystem::path( filename.toStdString() ) );
        boost::scoped_ptr< adcontrols::datafile > file( adcontrols::datafile::create( p.wstring() ) );
        return file && file->saveContents( L"/Processed", portfolio, this->file() );

    }
*/
    return true;
}

QString
ChemFile::fileName() const
{
    return filename_;
}

QString
ChemFile::defaultPath() const
{
    return "C:/Data";
}

QString
ChemFile::suggestedFileName() const
{
//	boost::filesystem::path path( file_->filename() );
//	path.replace_extension( L".sdf" );
//	return qtwrapper::qstring( path.wstring() );
	return "xyz.sdf";
}

bool
ChemFile::isReadOnly() const
{
//    if ( file_ && file_->readonly() )
//        return true;
//    return false;
	return true;
}

bool
ChemFile::isSaveAsAllowed() const
{
    return true;
}

void
ChemFile::modified( ReloadBehavior* behavior )
{
    Q_UNUSED(behavior);
}

///////////////////////////

//adcontrols::datafile&
//ChemFile::file()
//{
//    return *file_;
//}

