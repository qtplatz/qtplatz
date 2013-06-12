/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include <compiler/disable_dll_interface.h>
#include "smartspattern.hpp"
#include "mol.hpp"

#include <compiler/disable_unused_parameter.h>
#include <openbabel/parsmart.h>

using namespace adchem;

SmartsPattern::~SmartsPattern()
{
}

SmartsPattern::SmartsPattern() : obSmartsPattern_( new OpenBabel::OBSmartsPattern )
{
}

bool
SmartsPattern::init( const char * pattern )
{
    return obSmartsPattern_->Init( pattern );
}

bool
SmartsPattern::match( const Mol& mol, bool single )
{
    const OpenBabel::OBMol& omol = *(mol.obmol());
    return obSmartsPattern_->Match( const_cast< OpenBabel::OBMol& >(omol), single );
}

const OpenBabel::OBSmartsPattern *
SmartsPattern::get() const
{
    return obSmartsPattern_.get();
}

void
SmartsPattern::assign( const OpenBabel::OBSmartsPattern& t )
{
    obSmartsPattern_.reset( new OpenBabel::OBSmartsPattern( t ) );
}
