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

#include "sequencedocument.hpp"
#include "sequenceeditor.hpp"
#include "constants.hpp"

#include <adcontrols/processmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
//#include <adutils/adfsio.hpp>
#include <adfs/adfs.hpp>

#include <adportable/profile.hpp>
#include <adportable/serializer.hpp>
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

// debug
#include <adfs/sqlite3.h>

using namespace sequence;

SequenceDocument::~SequenceDocument()
{
}

SequenceDocument::SequenceDocument(  QObject *parent ) : QObject( parent )
{
    boost::filesystem::path dir( adportable::profile::user_data_dir<char>() );
    dir /= "data";
    defaultPath_ = dir.string().c_str();
    // filename_ = ( dir /= "default.sequ" ).string().c_str();
}

