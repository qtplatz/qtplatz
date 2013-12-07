/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "sdfile.hpp"
#include <adportable/debug.hpp>

#include <RDGeneral/Invariant.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <RDGeneral/RDLog.h>

using namespace chemistry;

SDFile::SDFile()
{
}

SDFile::SDFile( const std::string& filename, bool sanitize, bool removeHs, bool strictParsing )
    : molSupplier_( std::make_shared< RDKit::SDMolSupplier >( filename, sanitize, removeHs, strictParsing ) )
    , filename_( filename )
{
    adportable::debug(__FILE__, __LINE__) << filename_;
    adportable::debug(__FILE__, __LINE__) << molSupplier_->length();
}

// static
bool
SDFile::associatedData( const std::string& text, std::map< std::string, std::string >& data )
{
    data.clear();
    (void)text;
}
