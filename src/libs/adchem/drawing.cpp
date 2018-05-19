/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <RDGeneral/versions.h>

#if (RDKIT_VERSION <= RDKIT_VERSION_CHECK(2015, 9, 1))
# include <GraphMol/MolDrawing/MolDrawing.h>
# include <GraphMol/MolDrawing/DrawingToSVG.h>
#else
# include <GraphMol/MolDraw2D/MolDraw2DSVG.h>
#endif

using namespace adchem;

drawing::drawing()
{
}

// static
std::string
drawing::toSVG( const RDKit::ROMol& mol )
{
#if (RDKIT_VERSION <= RDKIT_VERSION_CHECK(2015, 9, 2))
    std::vector< int > drawing = RDKit::Drawing::MolToDrawing( mol );
    return RDKit::Drawing::DrawingToSVG( drawing );
#else // tried with 2018,9,1
    RDKit::ROMol mol1( mol );
    RDDepict::compute2DCoords( mol1 );
    std::ostringstream o;
    RDKit::MolDraw2DSVG svg_drawer( 300, 300, o );
    svg_drawer.drawMolecule( mol1 );
    svg_drawer.finishDrawing();
    return o.str();
#endif    
}
