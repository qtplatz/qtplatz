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

#include <GraphMol/MolDraw2D/MolDraw2DHelpers.h>
#if _MSC_VER
# pragma warning(disable:4267)
#endif
#include "drawing.hpp"
#include <adportable/debug.hpp>

#include <GraphMol/Depictor/RDDepictor.h>
#include <RDGeneral/versions.h>
#if defined WIN32 // workaround, which cause an error:
// Error	LNK2019	unresolved external symbol "class std::shared_ptr<class boost::logging::rdLogger> rdErrorLog" (? rdErrorLog@@3V ? $shared_ptr@VrdLogger@logging@boost@@@std@@A) referenced in function "public: static class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > __cdecl adchem::drawing::toSVG(class RDKit::ROMol const &)" (? toSVG@drawing@adchem@@SA ? AV ? $basic_string@DU ? $char_traits@D@std@@V ? $allocator@D@2@@std@@AEBVROMol@RDKit@@@Z)	adchem	C : \Users\toshi\src\build - x86_64\qtplatz.release\src\libs\adchem\drawing.obj	1
#include <RDGeneral/RDLog.h>
RDLogger rdErrorLog;
#endif

#if RDKIT_VERSION <= RDKIT_VERSION_CHECK(2015, 9, 1)
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
drawing::toSVG( const RDKit::ROMol& mol, RDKit::DrawColour&& background )
{
#if (RDKIT_VERSION <= RDKIT_VERSION_CHECK(2015, 9, 2))
    std::vector< int > drawing = RDKit::Drawing::MolToDrawing( mol );
    return RDKit::Drawing::DrawingToSVG( drawing );
#else

    if (rdErrorLog)
        ADDEBUG() << rdErrorLog.use_count();


    RDKit::ROMol mol1( mol );
    RDDepict::compute2DCoords( mol1 );

    std::ostringstream o;
    RDKit::MolDraw2DSVG svg_drawer( 300, 300, o );
    svg_drawer.drawOptions().backgroundColour = background;

    svg_drawer.drawMolecule( mol1 );
    svg_drawer.finishDrawing();
    return o.str();
#endif
}
