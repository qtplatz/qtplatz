/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "chemfile.hpp"

//#include <openbabel/obconversion.h>

#include <qtwrapper/qstring.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <QDebug>

using namespace chemistry;

ChemFile::~ChemFile()
{
    // adcontrols::datafile::close( file_ );
}

ChemFile::ChemFile( QObject * parent ) : Core::IFile( parent )
                                       , modified_( false )
                                       , nread_( 0 )
                                       , filesize_( 0 )
{
}

// bool
// ChemFile::open( const QString& qfilename, const OpenBabel::OBFormat * informat )
// {
// #if 0
//     nread_ = 0;
//     qfilename_ = qfilename;
// 	filename_ = static_cast< const char * >( qfilename_.toUtf8() );
//     if ( informat == 0 ) {
//         informat = OpenBabel::OBConversion::FormatFromExt( filename_.c_str() );
//         if ( informat == 0 ) {
//             qDebug() << "ChemFile: " << qfilename << " format from ext could not be identified";
//             return false;
//         }
//     }
    
//     if ( ! boost::filesystem::exists( filename_ ) ) {
//         qDebug() << "ChemFile: " << qfilename << " does not exist";
//         return false;
//     }

//     filesize_ = boost::filesystem::file_size( filename_ );
    
//     obconversion_->informat( const_cast< OpenBabel::OBFormat *>( informat ) );
// 	obconversion_->open( filename_.c_str() );
// #endif
//     return true;
// }

// unsigned long long
// ChemFile::tellg() const
// {
//     return 0;
// 	// return obconversion_->tellg();
// 	/*
//     std::istream * stream = obconversion_->GetInStream();
//     if ( stream )
// 	return stream->tellg();
//     return 0;
// 	*/
// }

// unsigned long long
// ChemFile::fsize() const
// {
//     return filesize_;
// }

// bool
// ChemFile::Read( adchem::Mol& mol )
// {
//     nread_++;
//     return obconversion_->read( mol );
// }

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
ChemFile::save( const QString& /* filename */ )
{
    return true;
}

QString
ChemFile::fileName() const
{
    return qfilename_;
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

