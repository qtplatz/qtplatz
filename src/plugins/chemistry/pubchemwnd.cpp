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
#include <QtCore/qjsondocument.h>
#include <QtGui/qtextdocument.h>
#include <adchem/drawing.hpp>
#include <adcontrols/chemicalformula.hpp>
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

#include <QBoxLayout>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QSvgRenderer>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextTable>
#include <QTextTableCell>
#include <QWidget>

#include <boost/algorithm/string/trim.hpp>
#include <boost/json/kind.hpp>
#include <boost/system.hpp>
#include <boost/json.hpp>
#include <map>
#include <memory>

namespace chemistry {

    class PubChemWnd::impl {
    public:
        std::unique_ptr< QTextDocument > textDocument_;
    };
}

using namespace chemistry;

PubChemWnd::~PubChemWnd()
{
    delete impl_;
}

PubChemWnd::PubChemWnd( QWidget * parent ) : QWidget( parent )
                                           , impl_( new impl{} )
{
    if ( auto layout = new QVBoxLayout( this ) ) {
        if ( auto edit =
             adwidgets::add_widget( layout, adwidgets::create_widget< QTextEdit >("pubchem") ) ) {
            impl_->textDocument_ = std::make_unique< QTextDocument >( edit );
            edit->setDocument( impl_->textDocument_.get() );
            edit->setAcceptRichText( true );
            edit->ensureCursorVisible();
            connect( edit, &QTextEdit::selectionChanged, this, [=](){
                auto cursor = edit->textCursor();
                auto block = cursor.block();
                //ADDEBUG() << "block.number: " << block.blockNumber() << ", line#" << block.firstLineNumber();
                ADDEBUG() << block.text().toStdString();
            });
        }
    }
}

void
PubChemWnd::handleReply( const QByteArray& ba, const QString& url )
{
    if ( auto edit = findChild< QTextEdit * >() ) {

        std::string smiles;
        boost::system::error_code ec;
        using adportable::json_helper;
        auto jv = boost::json::parse( ba.toStdString(), ec );
        if ( !ec ) {
            if ( auto p =
                 json_helper::find_pointer( jv, "/PropertyTable/Properties/0/CanonicalSMILES", ec ) ) {
                smiles = p->kind() == boost::json::kind::string ? p->as_string() : "";
            }
        }
        edit->moveCursor( QTextCursor::Start );

        auto cursor = edit->textCursor();
        cursor.insertBlock( QTextBlockFormat{} );
        cursor.insertText( QString("<url>%1</url>\n").arg( url ) );

        if ( auto table = cursor.insertTable( 1, 2 ) ) {
            {
                auto cell = table->cellAt( 0, 0 );
                auto cur = cell.firstCursorPosition();
                cur.insertText( QString::fromStdString( ba.toStdString() ) );
            }
#if HAVE_RDKit
            if ( !smiles.empty() ) {
                auto cell = table->cellAt( 0, 1 );
                auto cur = cell.firstCursorPosition();
                if ( auto mol =
                     std::unique_ptr< RDKit::RWMol >( RDKit::SmilesToMol( smiles ) ) ) {
                    std::string svg = adchem::drawing::toSVG( *mol );
                    QSvgRenderer renderer( QByteArray( svg.data(), svg.size() ) );
                    QImage image( 200, 200, QImage::Format_ARGB32 );
                    QPainter painter( &image );
                    renderer.render(&painter);
                    cur.insertImage( image );
                }
#endif
            }
        }
        cursor.movePosition( QTextCursor::Down );
        cursor.insertText( "\n----- END PUG REST -----\n" );
    }
}
