/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "drawing.hpp"
#if _MSC_VER
# pragma warning(disable:4267)
#endif

#include <GraphMol/Depictor/RDDepictor.h>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_sign_compare.h>

#include <GraphMol/MolDrawing/MolDrawing.h>
#include <compiler/disable_unused_variable.h>
#include <GraphMol/MolDrawing/DrawingToSVG.h>

#include <compiler/diagnostic_pop.h>

using namespace adchem;

drawing::drawing()
{
}

// static
std::string
drawing::toSVG( const RDKit::ROMol& mol )
{
    std::vector< int > drawing = RDKit::Drawing::MolToDrawing( mol );
    return RDKit::Drawing::DrawingToSVG( drawing );
}
