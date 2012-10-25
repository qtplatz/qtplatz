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
#if defined _MSC_VER
# pragma warning( disable: 4100 )
#endif
#include <openbabel/babelconfig.h>
#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#if defined _MSC_VER
# pragma warning( default: 4100 )
#endif

#include <qtwrapper/qstring.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

using namespace chemistry;

ChemFile::~ChemFile()
{
    // adcontrols::datafile::close( file_ );
}

ChemFile::ChemFile( QObject * parent ) : Core::IFile( parent )
	                                   , modified_( false )
									   , obconversion_( new OpenBabel::OBConversion() )
									   , nread_( 0 )
{
}

bool
ChemFile::open( const QString& qfilename, const OpenBabel::OBFormat * informat )
{
	nread_ = 0;
    qfilename_ = qfilename;
	filename_ = qfilename_.toStdString();
    if ( informat == 0 )
		informat = OpenBabel::OBConversion::FormatFromExt( filename_.c_str() );

	if ( informat ) {
		obconversion_->SetInFormat( const_cast< OpenBabel::OBFormat *>( informat ) );
		return true;
	}
	return false;
}

bool
ChemFile::Read( OpenBabel::OBMol& mol )
{
	if ( nread_++ == 0 )
		return obconversion_->ReadFile( &mol, filename_ );
	return obconversion_->Read( &mol );
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

