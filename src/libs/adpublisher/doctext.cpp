/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "doctext.hpp"
#include "document.hpp"
#include <xmlparser/pugixml.hpp>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QMessageBox>
#include <QTextCharFormat>
#include <QColor>
#include <qmargins.h>

namespace adpublisher {
    namespace detail {
        
        class docItemDelegate : public QStyledItemDelegate {
        public:
            
        };

        static const char* node_types[] = {
            "null", "document", "element", "pcdata", "cdata", "comment", "pi", "declaration"
        };

        class text_writer {
            QTextEdit& edit;
        public:
            text_writer( QTextEdit& e ) : edit( e ) {
            }

            void operator()( const pugi::xpath_node& node ) const {

                QString tag = QString( "<%1" ).arg( node.node().name() );
                for ( auto a : node.node().attributes() )
                    tag.append( QString( " %1=\"%2\"" ).arg( a.name(), a.value() ) );
                tag.append( ">" );

                auto color = edit.textColor();
                QTextCharFormat fmt;
                fmt.setForeground( QColor( "red" ) );
                edit.mergeCurrentCharFormat( fmt );
                edit.insertPlainText( tag );
                fmt.setForeground( color );
                edit.mergeCurrentCharFormat( fmt );
                //--------------------------------------
                
                auto font = fmt.font();
                if ( std::strcmp( node.node().name(), "title" ) == 0 )
                    fmt.setFontPointSize( 12 );
                else
                    fmt.setFontPointSize( 10 );
                edit.mergeCurrentCharFormat( fmt );

                edit.append( node.node().text().get() );

                fmt.setFont( font );
                edit.mergeCurrentCharFormat( fmt );

                for ( auto child : node.node().select_nodes( "./*" ) ) {
                    (*this)(child);
                }

                fmt.setForeground( QColor( "red" ) );
                edit.mergeCurrentCharFormat( fmt );
                edit.append( QString( "</%1>" ).arg( node.node().name() ) );
                fmt.setForeground( color );
                edit.mergeCurrentCharFormat( fmt );
            }
        };

    }
}

using namespace adpublisher;

docText::~docText()
{
}

docText::docText(QWidget *parent) : QTextEdit(parent)
{
}

void
docText::setDocument( std::shared_ptr< adpublisher::document >& t )
{
    doc_ = t;
    repaint( *(t->xml_document()) );
}

void
docText::repaint( const pugi::xml_document& doc )
{
    try {
        const pugi::xpath_node node = doc.select_single_node( "/article|/book" );
        
        detail::text_writer writer( *this );
        writer( node );
        
    } catch ( pugi::xpath_exception& ex ) {
        QMessageBox::warning( this, "adpublisher::docText", ex.what() );
    }

}

