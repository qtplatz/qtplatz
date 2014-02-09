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

#include "proteintable.hpp"
#include "mainwindow.hpp"
#include <adprot/protfile.hpp>
#include <adprot/protease.hpp>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QItemDelegate>
#include <QPainter>
#include <QTextDocument>
#include <QApplication>
#include <QKeyEvent>
#include <QClipboard>
#include <sstream>

namespace peptide {
    namespace detail {

        class ProteinTableDelegate : public QItemDelegate {
        public:
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                if ( index.column() == 1 ) {
                    render_sequence( painter, option, index.data().toString() );
                } else {
                    QItemDelegate::paint( painter, option, index );
                }
            }

            QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                if ( index.column() == 1 ) {
					return QSize( 800, 96 );
					/*
                    QTextDocument document;
					QTextOption to;
					to.setWrapMode( QTextOption::WordWrap );
					document.setDefaultTextOption( to );
                    document.setHtml( index.data().toString() );
					QSize size( document.size().width(), document.size().height() );
					return size;
					*/
                }
                return QItemDelegate::sizeHint( option, index );
            }
        private:
            void render_sequence( QPainter * painter, const QStyleOptionViewItem& option, const QString& text ) const {
                painter->save();
                QStyleOptionViewItemV4 op = option;
                QTextDocument document;
                QTextOption to;
                to.setWrapMode( QTextOption::WrapAtWordBoundaryOrAnywhere );
                document.setDefaultTextOption( to );
				QFont font;
				font.setFamily( "Consolas" );
				document.setDefaultFont( font );
				document.setTextWidth( op.rect.width() );
                document.setHtml( text );
                op.widget->style()->drawControl( QStyle::CE_ItemViewItem, &op, painter );
                painter->translate( op.rect.topLeft() );
                QRect clip( 0, 0, op.rect.width(), op.rect.height() );
                document.drawContents( painter, clip );
                painter->restore();
            }
        };

    }
}

using namespace peptide;

ProteinTable::ProteinTable(QWidget *parent) : QTableView( parent )
                                            , model_( new QStandardItemModel )
                                            , delegate_( new detail::ProteinTableDelegate )
{
    setModel( model_ );
    setItemDelegate( delegate_ );
    init( *model_ );
}

ProteinTable::~ProteinTable()
{
    delete delegate_;
    delete model_;
}

void
ProteinTable::init( QStandardItemModel& model )
{
    model.setColumnCount( 2 );
    model.setHeaderData( 0, Qt::Horizontal, QObject::tr("name") );
    model.setHeaderData( 1, Qt::Horizontal, QObject::tr("sequence") );
	setColumnWidth( 0, 200 );
    setColumnWidth( 1, 500 );
	setWordWrap( true );
}

void
ProteinTable::setData( const adprot::protfile& file ) 
{
    QStandardItemModel& model = *model_;

    adprot::protease& trypsin = *MainWindow::instance()->get_protease();

    model.setRowCount( static_cast<int>( file.size() ) );
    int row = 0;
    for ( auto& prot: file ) {
        model.setData( model.index( row, 0 ), QString::fromStdString( prot.name() ) );

        std::string worded, richText;
        split( prot.sequence(), worded );
        adprot::protease::digest( trypsin, worded, richText );
        model.setData( model.index( row, 1 ), QString::fromStdString( richText ) );

        model.item( row, 0 )->setEditable( false );
        model.item( row, 1 )->setEditable( false );

		++row;
    }
    resizeRowsToContents();
}

void
ProteinTable::split( const std::string& sequence, std::string& worded )
{
    const size_t width = 10;
    size_t pos = 0;
    while ( pos < sequence.size() ) {
        worded += sequence.substr( pos, width );
        worded += "\n"; // for word wrap
        pos += width;
    }

}

void
ProteinTable::currentChanged( const QModelIndex& index, const QModelIndex& prev )
{
    emit currentChanged( index.row() );
}

void
ProteinTable::keyPressEvent( QKeyEvent * event )
{
    if ( event->matches( QKeySequence::Copy ) ) {
        handleCopyToClipboard();
    }
    // else if ( event->matches( QKeySequence::Paste ) ) {
    //     handlePasteFromClipboard();
     else {
        QTableView::keyPressEvent( event );
    }
}

void
ProteinTable::handleCopyToClipboard()
{
    QStandardItemModel& model = *model_;
    QModelIndexList list = selectionModel()->selectedIndexes();

    qSort( list );
    if ( list.size() < 1 )
        return;

    QString copy_table;
    QModelIndex prev = list.first();
	int i = 0;
    for ( auto idx: list ) {
		if ( i++ > 0 )
			copy_table.append( prev.row() == idx.row() ? '\t' : '\n' );
        copy_table.append( model.data( idx ).toString() );
        prev = idx;
    }
    QApplication::clipboard()->setText( copy_table );
}
