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

#include "docedit.hpp"
#include "document.hpp"
#include <adportable/debug.hpp>
#include <xmlparser/pugixml.hpp>
#include <QAbstractItemView>
#include <QCompleter>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QColor>
#include <qmargins.h>
#include <array>
#include <cstring>
#include <functional>
#include <fstream>
#include <regex>
#include <map>

namespace adpublisher {

    namespace detail {

        class docItemDelegate : public QStyledItemDelegate {
        public:
            
        };

        static const char* node_types[] = {
            "null", "document", "element", "pcdata", "cdata", "comment", "pi", "declaration"
        };

        class text_writer {
            docEdit& edit;

            QTextCharFormat plainFormat;
            QTextCharFormat paragraphFormat;
            QTextCharFormat headingFormat;
            QTextCharFormat empasisFormat;
            QTextCharFormat tagFormat;
            QTextCharFormat underlineFormat;
            QTextFrameFormat frameFormat;
            QTextFrame * mainFrame;

        public:
            text_writer( docEdit& e ) : edit( e ) {
                QTextCursor cursor = edit.textCursor();
                cursor.movePosition( QTextCursor::Start );
                mainFrame = cursor.currentFrame();

                plainFormat = cursor.charFormat();
                plainFormat.setFontPointSize( 10 );

                paragraphFormat = plainFormat;
                paragraphFormat.setFontPointSize( 12 );

                headingFormat = plainFormat;
                headingFormat.setFontWeight( QFont::Bold );
                headingFormat.setFontPointSize( 16 );

                empasisFormat = plainFormat;
                empasisFormat.setFontItalic( true );

                tagFormat = plainFormat;
                tagFormat.setForeground( QColor( "#990000" ) );
                tagFormat.setFontUnderline( true );
                
                underlineFormat = plainFormat;
                underlineFormat.setFontUnderline( true );

                frameFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Inset );
                frameFormat.setBorder( 1 );
                frameFormat.setMargin( 10 );
                frameFormat.setPadding( 4 );
            }

            static QString tagString( const pugi::xpath_node& node ) {
                QString tag = QString( "<%1" ).arg( node.node().name() );
                for ( auto a : node.node().attributes() )
                    tag.append( QString( " %1=\"%2\"" ).arg( a.name(), a.value() ) );
                tag.append( ">" );                
                return tag;
            }

            void operator()( const pugi::xpath_node& node, QTextFrame * pframe = 0, int level = 0 ) const {

                QTextCursor cursor = edit.textCursor();
                if ( pframe )
                    cursor = pframe->lastCursorPosition();

                cursor.insertText( tagString( node ), tagFormat ); // <----------- <tag>

                auto frame = cursor.insertFrame( frameFormat );    // <-------------- insert frame
                QObject::connect( frame, &QTextFrame::destroyed, &edit, &docEdit::handleBlockDeleted );
                
                // body
                if ( std::strcmp( node.node().name(), "title" ) == 0 ) {

                    cursor.insertText( QString::fromStdString( node.node().text().get() ), headingFormat );
                }
                else if ( std::strcmp( node.node().name(), "paragraph" ) == 0 ) {

                    cursor.insertText( QString::fromStdString( node.node().text().get() ), paragraphFormat );
                }
                else {

                    cursor.insertText( QString::fromStdString( node.node().text().get() ), plainFormat );
                }

                // process childlen 
                for ( auto& child : node.node().select_nodes( "./*" ) ) {
                    (*this)(child, frame, level + 1);
                }
                cursor = pframe ? pframe->lastCursorPosition() : mainFrame->lastCursorPosition();
                //cursor.insertText( QString( "</%1>" ).arg( node.node().name() ), tagFormat ); // </tag>
            }
        };



        class xhtml_writer {
            docEdit& edit;

            QTextCharFormat plainFormat;
            QTextCharFormat paragraphFormat;
            QTextCharFormat headingFormat;
            QTextCharFormat empasisFormat;
            QTextCharFormat tagFormat;
            QTextCharFormat underlineFormat;
            QTextFrameFormat frameFormat;
            QTextFrame * mainFrame;

        public:
            xhtml_writer( docEdit& e ) : edit( e ) {
                QTextCursor cursor = edit.textCursor();
                cursor.movePosition( QTextCursor::Start );
                mainFrame = cursor.currentFrame();

                plainFormat = cursor.charFormat();
                plainFormat.setFontPointSize( 10 );

                paragraphFormat = plainFormat;
                paragraphFormat.setFontPointSize( 12 );

                headingFormat = plainFormat;
                headingFormat.setFontWeight( QFont::Bold );
                headingFormat.setFontPointSize( 16 );

                empasisFormat = plainFormat;
                empasisFormat.setFontItalic( true );

                tagFormat = plainFormat;
                tagFormat.setForeground( QColor( "#990000" ) );
                tagFormat.setFontUnderline( true );
                
                underlineFormat = plainFormat;
                underlineFormat.setFontUnderline( true );

                frameFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Inset );
                frameFormat.setBorder( 1 );
                frameFormat.setMargin( 10 );
                frameFormat.setPadding( 4 );
            }

            void operator()( const pugi::xpath_node& node, int level = 0 ) const {
                QTextCursor cursor = edit.textCursor();
                QString name( node.node().name() );
                if ( name != "svg" ) {
                    QString text( node.node().text().get() );
                    if ( !text.isEmpty() ) {
                        // cursor.insertText( name, paragraphFormat );
                        cursor.insertText( text, paragraphFormat );
                    }
                    // process childlen
                    for ( auto& child : node.node().select_nodes( "./*" ) ) {
                        (*this)(child, level + 1);
                    }
                }
            }
        };

    }
}

using namespace adpublisher;

docEdit::~docEdit()
{
}

docEdit::docEdit(QWidget *parent) : QTextEdit(parent)
                                  , completer_(0)
{
}

void
docEdit::setDocument( std::shared_ptr< adpublisher::document >& t )
{
    doc_ = t;
    clear();
    repaint( *(t->xml_document()) );
}

void
docEdit::repaint( const pugi::xml_document& doc )
{
    if ( const pugi::xpath_node node = doc.select_single_node( "/article|/book" ) ) {
        try {
            detail::text_writer writer( *this );
            writer( node );
            auto cursor = textCursor();
            cursor.movePosition( QTextCursor::Start );
            ensureCursorVisible();
        } catch ( pugi::xpath_exception& ex ) {
            QMessageBox::warning( this, "adpublisher::docEdit", ex.what() );
        }
    }
    else if ( const pugi::xpath_node node = doc.select_single_node( "/qtplatz_document" ) ) {
        // do nothing
    }
    else {
        try {
            detail::xhtml_writer writer( *this );
            writer( doc.select_single_node( "/" ) );
            auto cursor = textCursor();
            cursor.movePosition( QTextCursor::Start );
            ensureCursorVisible();
        }
        catch ( pugi::xpath_exception& ex ) {
            QMessageBox::warning( this, "adpublisher::docEdit", ex.what() );
        }
    }
}

void
docEdit::setCompleter(QCompleter *completer)
{
    if (completer_)
        QObject::disconnect(completer_, 0, this, 0);

    completer_ = completer;

    if (!completer_)
        return;

    completer_->setWidget(this);
    completer_->setCompletionMode(QCompleter::PopupCompletion);
    completer_->setCaseSensitivity(Qt::CaseInsensitive);
    connect( completer_, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::activated), this, &docEdit::insertCompletion );
}

QCompleter *
docEdit::completer() const
{
    return completer_;
}

void
docEdit::insertCompletion(const QString& completion)
{
    if (completer_->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - completer_->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString
docEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void
docEdit::focusInEvent(QFocusEvent *e)
{
    if (completer_)
        completer_->setWidget( this );
    QTextEdit::focusInEvent(e);
}

void
docEdit::keyPressEvent(QKeyEvent *e)
{
    if ( completer_ && completer_->popup()->isVisible() ) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if ( !completer_ || !isShortcut ) // do not process the shortcut when we have a completer
        QTextEdit::keyPressEvent( e );

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!completer_ || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
                      || eow.contains(e->text().right(1)))) {
        completer_->popup()->hide();
        return;
    }

    if (completionPrefix != completer_->completionPrefix()) {
        completer_->setCompletionPrefix( completionPrefix );
        completer_->popup()->setCurrentIndex( completer_->completionModel()->index( 0, 0 ) );
    }
    QRect cr = cursorRect();
    cr.setWidth( completer_->popup()->sizeHintForColumn( 0 )
                 + completer_->popup()->verticalScrollBar()->sizeHint().width() );
    completer_->complete( cr ); // popup it up!
}


void
docEdit::contextMenuEvent( QContextMenuEvent * e )
{
    std::vector< std::pair< QAction *, std::function<void()> > > items;
    if ( QMenu * menu = createStandardContextMenu( e->globalPos() ) ) {

        menu->addAction( tr( "Add Summary Table" ), this, SLOT( addSummaryTable() ) );

        menu->exec( e->globalPos() );
    }
}

void
docEdit::addSummaryTable()
{
    QTextCursor cursor = textCursor();
    cursor.insertText( QString( QChar::ObjectReplacementCharacter ) );
}

void
docEdit::handleBlockDeleted()
{
}

void
docEdit::fetch( pugi::xml_document& xml )
{
    QString article_title, article_author;
    std::map< std::string, std::pair< QString, QString > > sections;

    auto cursor = document()->find( "<article>" ); // find a top of the 'article'
    if ( !cursor.isNull() ) {
        auto block = cursor.block().next();
        if ( block.isValid() ) {
            if ( block.text().contains( "<title lang=\"en-us\">" ) ) {
                block = block.next();
                if ( block.isValid() )
                    article_title = block.text();
            }
        }
    }

    cursor = document()->find( "<author" ); // find 'author'
    if ( ! cursor.isNull() ) {
        auto block = cursor.block().next();
        if ( block.isValid() ) {
            article_author = block.text();
        }
    }
    cursor = document()->find( "<section" ); // find sections
    while ( ! cursor.isNull() ) {

        std::string section_id;
        QString section_title, paragraph;
        auto block = cursor.block();
        if ( block.isValid() ) {
            std::string text = block.text().toStdString();

            std::regex regex( "<section[ \t]*id=\"(.*)\"[ \t]+.*" );
            std::smatch match;
            if ( std::regex_match( text, match, regex ) ) {
                if ( match.size() >= 2 )
                    section_id = match[ 1 ].str();
            }
        }

        if ( ( block = block.next() ).isValid() ) {
            if ( block.text().contains( "<title lang=" ) ) {
                if ( ( block = block.next() ).isValid() )
                    section_title = block.text();
                if ( ( block = block.next() ).isValid() ) {
                    if ( block.text().contains( "<paragraph lang=" ) ) {
                        if ( ( block = block.next() ).isValid() ) {
                            paragraph = block.text();
                        }
                    }
                }
            }
        }

        sections[ section_id ] = std::make_pair( section_title, paragraph );

        cursor = document()->find( "<section", cursor ); // next
    }
    
    auto doc = xml.document_element();
    if ( auto node = doc.select_single_node( "/article/title" ) ) {
        node.node().text() = static_cast<const char *>( article_title.toUtf8() );
    }
    if ( auto node = doc.select_single_node( "/article/author" ) ) {
        node.node().text() = static_cast<const char *>( article_author.toUtf8() );
    }
    for ( auto& sec : sections ) {
        QString query_title = QString( "/article/section[@id='%1']/title" ).arg( sec.first.c_str() );
        QString query_para = QString( "/article/section[@id='%1']/paragraph" ).arg( sec.first.c_str() );

        if ( auto node = doc.select_single_node(static_cast< const char *>( query_title.toUtf8() ) ) ) {
            node.node().text() = static_cast<const char *>( sec.second.first.toUtf8() );
        }
        if ( auto node = doc.select_single_node(static_cast< const char *>( query_para.toUtf8() ) ) ) {
            node.node().text() = static_cast<const char *>( sec.second.second.toUtf8() );
        }
    }


}