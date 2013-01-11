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
#include <adportable/profile.hpp>
#include <adsequence/sequence.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <qtwrapper/qstring.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <qmessagebox.h>

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
    boost::filesystem::path path = qtwrapper::wstring::copy( filename );

    std::ifstream inf( path.string().c_str() );

    try {
        // portable_binary_iarchive ar( inf );
        boost::archive::xml_iarchive ar( inf );
        ar >> BOOST_SERIALIZATION_NVP(*adsequence_);
    } catch ( std::exception& ex ) {
        QMessageBox::warning( 0, "SequenceFile", "FILE OPEN FAILED" );
    }

    if ( ! filename.isEmpty() )
        filename_ = filename;

    setModified( false );
    return true;
}

bool
SequenceFile::save( const QString& filename )
{
    editor_.getSequence( *adsequence_ );

    if ( ! filename.isEmpty() ) // save as
        filename_ = filename; // replace filename

    boost::filesystem::path path( qtwrapper::wstring::copy( filename_ ) );
    std::ofstream outf( path.string().c_str() );

    try {
        // portable_binary_oarchive ar( outf );
        boost::archive::xml_oarchive ar( outf );
        ar << BOOST_SERIALIZATION_NVP(*adsequence_);
        setModified( false );
    } catch ( std::exception& ex ) {
        QMessageBox::warning( 0, "SequenceFile", "FILE SAVE FAILED" );
    }
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
