/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "moldraw.hpp"

#if defined HAVE_RDKit && HAVE_RDKit
#if defined _MSC_VER
# pragma warning( disable: 4267 4018 )
#endif

#include <RDGeneral/Invariant.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <RDGeneral/RDLog.h>
#if _MSC_VER
#include <GraphMol/MolDraw2D/MolDraw2DSVG.h> // <-- if RDKit installs in INTREE
#else
#include <MolDraw2DSVG.h>
#endif
#endif

using namespace adwidgets;

std::string
MolDraw::toSVG( int w, int h, RDKit::ROMol& mol )
{
#if HAVE_RDKit
    mol.updatePropertyCache( false );
    RDDepict::compute2DCoords( mol );
    RDKit::MolDraw2DSVG drawer( w, h );
    drawer.drawMolecule( mol );
    drawer.finishDrawing();
    return drawer.getDrawingText();
#else
    return std::string();
#endif
}

