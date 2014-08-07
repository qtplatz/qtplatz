// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include <adportable/profile.hpp>
#include <qtwrapper/qstring.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <qmessagebox.h>

using namespace dataproc;

IFileImpl::~IFileImpl()
{
    adcontrols::datafile::close( file_ );
}


IFileImpl::IFileImpl( adcontrols::datafile * file
                     , Dataprocessor& dprocessor
                     , QObject *parent) : Core::IDocument(parent)
                                        , modified_(false)
                                        , file_(file)
                                        , accessor_(0)
                                        , dprocessor_( dprocessor ) 
{
	if ( file_ ) {
		boost::filesystem::path path( file_->filename() );
		if ( path.extension() != L".adfs" )
			modified_ = true;
	}
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
IFileImpl::save( QString *, const QString& filename, bool )
{
    portfolio::Portfolio portfolio = dprocessor_.getPortfolio();

	boost::filesystem::path path( file_->filename() ); // original name
	if ( path.extension() != L".adfs" )
		path.replace_extension( L".adfs" );

    if ( filename.isEmpty() || ( path == boost::filesystem::path( filename.toStdString() ) ) ) {
        // Save
        if ( ! this->file().saveContents( L"/Processed", portfolio ) )
			return false;

    } else { // save as 'filename

        path = filename.toStdWString();

		if ( boost::filesystem::exists( path ) ) {
            QMessageBox::warning( 0, "Datafile save", "file already exists" );
            return false;
        }
        std::unique_ptr< adcontrols::datafile > file( adcontrols::datafile::create( path.wstring() ) );
        if ( ! ( file && file->saveContents( L"/Processed", portfolio, this->file() ) ) )
			return false;
    }

    // for debug convension
    do {
        path.replace_extension( ".xml" );
        boost::filesystem::remove( path );
        pugi::xml_document dom;
        dom.load( portfolio.xml().c_str() );
        dom.save_file( path.string().c_str() );
    } while(0);

	setModified( false );
    return true;
}

bool
IFileImpl::reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType )
{
    return true;
}

QString
IFileImpl::fileName() const
{
	return QString::fromStdWString( file_->filename() );
}

QString
IFileImpl::defaultPath() const
{
	return adportable::profile::user_data_dir<char>().c_str();
}

QString
IFileImpl::suggestedFileName() const
{
	boost::filesystem::path path( file_->filename() );
	path.replace_extension( L".adfs" );
	return qtwrapper::qstring( path.wstring() );
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
    return true;
}

bool
IFileImpl::subscribe( const adcontrols::ProcessedDataset& processed )
{
    std::string xml = processed.xml();
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

