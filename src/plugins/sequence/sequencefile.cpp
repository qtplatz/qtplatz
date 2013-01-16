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

#include "sequencefile.hpp"
#include "sequenceeditor.hpp"
#include "constants.hpp"
#include "serializer.hpp"
#include <adcontrols/processmethod.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adinterface/controlmethodC.h>
#include <adportable/profile.hpp>
#include <adsequence/sequence.hpp>
//
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <qtwrapper/qstring.hpp>
#include <qmessagebox.h>
#include <fstream>
#include <iostream>

using namespace sequence;

SequenceFile::~SequenceFile()
{
}

SequenceFile::SequenceFile( const SequenceEditor& editor
                            , QObject *parent ) : Core::IFile( parent )
                                                , editor_( editor )
                                                , mimeType_( sequence::Constants::C_SEQUENCE_MIMETYPE )
                                                , modified_( false )
                                                , adsequence_( new adsequence::sequence )
{
    boost::filesystem::path dir( adportable::profile::user_data_dir<char>() );
    dir /= "data";
    defaultPath_ = dir.string().c_str();
    // filename_ = ( dir /= "default.sequ" ).string().c_str();
}

void
SequenceFile::setModified( bool val )
{
    modified_ = val;
    emit changed();
}

bool
SequenceFile::isModified() const
{
    return modified_;
}

QString
SequenceFile::mimeType() const
{
	return mimeType_;
}

bool
SequenceFile::load( const QString& filename )
{
    ctrlmethods_.clear();
    procmethods_.clear();

    boost::filesystem::path path = qtwrapper::wstring::copy( filename );
    adfs::portfolio file;
    try {
        if ( ! file.mount( path.wstring().c_str() ) )
            return false;
    } catch ( adfs::exception& ex ) {
        QMessageBox::warning( 0, "SequenceFile", (boost::format("%1% on %2%") % ex.message % ex.category ).str().c_str() );
        return false;
    }

    do {
        // .sequ file should have only one folium under /Sequence folder
        adfs::folder folder = file.findFolder( L"/Sequence" );
        std::vector< adfs::folium > folio = folder.folio();
        if ( folio.empty() )
            return false;
        adfs::folium& folium = *folio.begin();  // take first one.
        adfs::cpio< adsequence::sequence >::copyout( *adsequence_, folium );
    } while( 0 );

    do {
        adfs::folder folder = file.findFolder( L"/ProcessMethod" );

        std::vector< adfs::folium > folio = folder.folio();
        for ( std::vector< adfs::folium >::iterator it = folio.begin(); it != folio.end(); ++it ) {
            boost::shared_ptr< adcontrols::ProcessMethod > ptr( new adcontrols::ProcessMethod );
            adfs::cpio< adcontrols::ProcessMethod >::copyout( *ptr, *it );
            procmethods_[ it->name() ] = ptr;
        }
    } while ( 0 );
    
    do {
        adfs::folder folder = file.addFolder( L"/ControlMethod" );

        std::vector< adfs::folium > folio = folder.folio();
        for ( std::vector< adfs::folium >::iterator it = folio.begin(); it != folio.end(); ++it ) {
            std::vector< char> ibuf( it->size() );
            it->read( ibuf.size(), &ibuf[0] );
            boost::shared_ptr< ControlMethod::Method > ptr( new ControlMethod::Method );
            serializer::restore( *ptr, ibuf );
            // dubug
            ptr->subject = CORBA::wstring_dup( it->name().c_str() );
            //
            ctrlmethods_[ it->name() ] = ptr;
            std::wcout << L"Loading control method: ['" << ptr->subject.in() << L"'] has " 
                << ptr->lines.length() << " lines in " 
                << it->size() << " bytes"
                << std::endl;
        }
    } while ( 0 );

    if ( ! filename.isEmpty() )
        filename_ = filename;
    setModified( true );
    return true;
}

bool
SequenceFile::save( const QString& filename )
{
    editor_.getSequence( *adsequence_ );
    
    if ( ! filename.isEmpty() ) // save as
        filename_ = filename; // replace filename

    boost::filesystem::path path( qtwrapper::wstring::copy( filename_ ) );
    path.replace_extension( ".sequ" );

    adfs::portfolio file;
    try {
        if ( ! file.create( path.wstring().c_str() ) )
            return false;
    } catch ( adfs::exception& ex ) {
        QMessageBox::warning( 0, "SequenceFile", (boost::format("%1% on %2%") % ex.message % ex.category ).str().c_str() );
        return false;
    }

    do {
        adfs::folder folder = file.addFolder( L"/Sequence" );
        adfs::folium folium = folder.addFolium( adfs::create_uuid() );
        adfs::cpio< adsequence::sequence >::copyin( *adsequence_, folium );
        folium.dataClass( L"adsequence::sequence" );
        folium.commit();
    } while( 0 );

    do {
        adfs::folder folder = file.addFolder( L"/ProcessMethod" );

        for ( process_method_map_type::const_iterator it = procmethods_.begin(); it != procmethods_.end(); ++it ) {
            adfs::folium folium = folder.addFolium( it->first );
            adfs::cpio< adcontrols::ProcessMethod >::copyin( *it->second, folium );
            folium.dataClass( L"adcontrols::ProcessMethod" );
            folium.commit();
        }
    } while ( 0 );
    
    do {
        adfs::folder folder = file.addFolder( L"/ControlMethod" );
        for ( control_method_map_type::const_iterator it = ctrlmethods_.begin(); it != ctrlmethods_.end(); ++it ) {
            adfs::folium folium = folder.addFolium( it->first );
            
            std::vector< char > obuf;
            serializer::archive( obuf, *it->second );
            folium.write( obuf.size(), &obuf[0] );

            folium.dataClass( L"ControlMethod::Method" );
            folium.commit();

            std::wcout << L"Saving control method: ['" << it->second->subject.in() << L"'] has "
                << it->second->lines.length() << " lines in "
                << obuf.size() << " bytes"
                << std::endl;

        }
    } while ( 0 );
    
#if 0 // old code
    std::ofstream outf( path.string().c_str() );
	if ( ! adsequence::sequence::xml_archive( outf, *adsequence_ ) )
            QMessageBox::warning( 0, "SequenceFile", ( boost::format( "FILE %1% SAVE FAILED" ) % path.string() ).str().c_str() );
#endif

	return true;
}

QString
SequenceFile::fileName() const
{
    return filename_;
}

QString
SequenceFile::defaultPath() const
{
    return defaultPath_;
}

QString
SequenceFile::suggestedFileName() const
{
    return QString();
}

bool
SequenceFile::isReadOnly() const
{
    return false;
}

bool
SequenceFile::isSaveAsAllowed() const
{
    return true;
}

void
SequenceFile::modified( ReloadBehavior* behavior )
{
    Q_UNUSED(behavior);
}

adsequence::sequence&
SequenceFile::adsequence()
{
    return * adsequence_;
}

const adsequence::sequence&
SequenceFile::adsequence() const
{
    return * adsequence_;
}

void
SequenceFile::removeProcessMethod( const std::wstring& name )
{
	process_method_map_type::iterator it = procmethods_.find ( name );
    if ( it != procmethods_.end() )
		procmethods_.erase( it );
}

void
SequenceFile::removeControlMethod( const std::wstring& name )
{
	control_method_map_type::iterator it = ctrlmethods_.find ( name );
	if ( it != ctrlmethods_.end() )
		ctrlmethods_.erase( it );
}

const adcontrols::ProcessMethod *
SequenceFile::getProcessMethod( const std::wstring& name ) const
{
	process_method_map_type::const_iterator it = procmethods_.find( name );
	if ( it == procmethods_.end() )
		return 0;
	return it->second.get();
}

const ControlMethod::Method *
SequenceFile::getControlMethod( const std::wstring& name ) const
{
	control_method_map_type::const_iterator it = ctrlmethods_.find( name );
	if ( it == ctrlmethods_.end() )
		return 0;
	return it->second.get();
}

void
SequenceFile::setProcessMethod( const std::wstring& name, const adcontrols::ProcessMethod& m )
{
	using adcontrols::ProcessMethod;
	procmethods_[ name ] = boost::shared_ptr< ProcessMethod >( new ProcessMethod( m ) );
}

void
SequenceFile::setControlMethod( const std::wstring& name, const ControlMethod::Method& m )
{
	using ControlMethod::Method;
    ctrlmethods_[ name ] = boost::shared_ptr< Method >( new Method( m ) );
}

