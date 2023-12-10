/**************************************************************************
** Copyright (C) 2014-2024 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "pubchemwnd.hpp"
#include <QtSvg/qsvgrenderer.h>
#include <adchem/drawing.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <QtCore/qjsondocument.h>
#include <QtGui/qtextcursor.h>
#include <adportable/debug.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <adwidgets/create_widget.hpp>
#if HAVE_RDKit
# include <GraphMol/SmilesParse/SmilesParse.h>
# include <GraphMol/RDKitBase.h>
# include <GraphMol/SmilesParse/SmilesParse.h>
# include <GraphMol/SmilesParse/SmilesWrite.h>
# include <GraphMol/Descriptors/MolDescriptors.h>
# include <GraphMol/FileParsers/MolSupplier.h>
# include <GraphMol/inchi.h>
#endif

#include <QWidget>
#include <QTextEdit>
#include <QBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QImage>
#include <QSvgRenderer>

#include <boost/json/kind.hpp>
#include <boost/system.hpp>
#include <boost/json.hpp>
#include <map>

using namespace chemistry;

PubChemWnd::~PubChemWnd()
{
}

PubChemWnd::PubChemWnd( QWidget * parent ) : QWidget( parent )
{
    if ( auto layout = new QVBoxLayout( this ) ) {
        if ( auto edit = adwidgets::add_widget( layout, adwidgets::create_widget< QTextEdit >("pubchem") ) ) {
            edit->setAcceptRichText( true );
            edit->ensureCursorVisible();
        }
    }
}

void
PubChemWnd::handleReply( const QByteArray& ba )
{
    if ( auto edit = findChild< QTextEdit * >() ) {
        edit->moveCursor( QTextCursor::Start );

        std::string smiles;
        boost::system::error_code ec;
        auto jv = boost::json::parse( ba.toStdString(), ec );
        if ( !ec ) {
            // rfc6901 (JSON pointer), require boost-1.81 or higher
#if BOOST_VERSION >= 108100
            if ( auto p = jv.find_pointer("/PropertyTable/Properties/0/CanonicalSMILES", ec) ) {
                smiles = p->kind() == boost::json::kind::string ? p->as_string() : "";
            }
#else
            if ( auto p = adportable::json_helper::if_contains( jv, "/PropertyTable/Properties/0/CanonicalSMILES" ) ) {
                smiles = p->kind() == boost::json::kind::string ? p->as_string() : "";
            }
#endif
        }

        auto cursor = edit->textCursor();
        cursor.insertText( QString::fromStdString( ba.toStdString() ) );
        cursor.insertText( "\n" );

        if ( !smiles.empty() ) {
#if HAVE_RDKit
            if ( auto mol = std::unique_ptr< RDKit::RWMol >( RDKit::SmilesToMol( smiles ) ) ) {
                std::string svg = adchem::drawing::toSVG( *mol );
                QSvgRenderer renderer( QByteArray( svg.data(), svg.size() ) );
                QImage image( 200, 200, QImage::Format_ARGB32 );
                QPainter painter( &image );
                renderer.render(&painter);
                // QString img = QString( R"(<img src=%1 width="100" height=100">)" ).arg( QString::fromStdString( svg.c_str() ) );
                cursor.insertImage( image );
            }
#else
            cursor.insertText( QString("CanonicalSMILES = %1").arg( QString::fromStdString( smiles ) ) );
#endif
            cursor.insertText( "\n------ END OF PUG REST -----\n" );
        }

    }
}
