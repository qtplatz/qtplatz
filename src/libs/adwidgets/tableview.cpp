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

#include "tableview.hpp"
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QList>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <set>

using namespace adwidgets;

TableView::TableView(QWidget *parent) : QTableView(parent)
                                      , allowDelete_( true )
{
    verticalHeader()->setDefaultSectionSize( 18 );
}

void
TableView::keyPressEvent( QKeyEvent * event )
{
	if ( event->matches( QKeySequence::Copy ) ) {
        handleCopyToClipboard();
    } else if ( event->matches( QKeySequence::Paste ) ) {
        handlePaste();
    } else if ( event->matches( QKeySequence::Delete ) && allowDelete_ ) {
		handleDeleteSelection();
	} else
		QTableView::keyPressEvent( event );
}

void
TableView::mouseReleaseEvent( QMouseEvent * event )
{
	QModelIndex index = indexAt( event->pos() );
	QTableView::mousePressEvent( event );

	if ( index.isValid() ) {
        Qt::ItemFlags flags = model()->flags( index );
        if ( flags & Qt::ItemIsUserCheckable ) {
            QVariant st = index.data( Qt::CheckStateRole );
			if ( index.data( Qt::EditRole ).type() == QVariant::Bool )
                model()->setData( index, ( st == Qt::Checked ) ? true : false );
        }
    }
}

void 
TableView::handlePaste()
{
}

void
TableView::handleCopyToClipboard()
{
	QModelIndexList indecies = selectionModel()->selectedIndexes();

    qSort( indecies );
    if ( indecies.size() < 1 )
        return;

    QString selected_text;
    QModelIndex prev = indecies.first();
    QModelIndex last = indecies.last();

    indecies.removeFirst();
    for( int i = 0; i < indecies.size(); ++i ) {
        QModelIndex index = indecies.at( i );

        if ( !isRowHidden( prev.row() ) ) {

            //auto t = prev.data( Qt::EditRole ).type();
            if ( !isColumnHidden( prev.column() ) ) {

                QString text = prev.data( Qt::EditRole ).toString();
                selected_text.append( text );

                if ( index.row() == prev.row() )
                    selected_text.append( '\t' );
            }
            
            if ( index.row() != prev.row() )
                selected_text.append( '\n' );
        }
        prev = index;
    }

    if ( !isRowHidden( last.row() ) && !isColumnHidden( last.column() ) )
        selected_text.append( last.data( Qt::EditRole ).toString() );

    QMimeData * md = new QMimeData();
    md->setText( selected_text );
    QApplication::clipboard()->setMimeData( md );
}

void
TableView::handleDeleteSelection()
{
	QModelIndexList indecies = selectionModel()->selectedIndexes();

    qSort( indecies );
    if ( indecies.size() < 1 )
        return;

	std::set< int > rows;
    for( int i = 0; i < indecies.size(); ++i ) {
        QModelIndex index = indecies.at( i );
		rows.insert( index.row() );
	}
	std::vector< std::pair< int, int > > ranges;

	for ( auto it = rows.begin(); it != rows.end(); ++it ) {
		std::pair< int, int > range;
		range.first = *it;
		range.second = *it;
		while ( ++it != rows.end() && *it == range.second + 1 )
			range.second = *it;
		--it;
		ranges.push_back( range );
	}

	// remove from botton to top
	for ( auto range = ranges.rbegin(); range != ranges.rend(); ++range )
		model()->removeRows( range->first, range->second - range->first + 1 );

    emit rowsDeleted();
}

void
TableView::addActionsToMenu( QMenu& menu, const QPoint& )
{
    menu.addAction( tr( "Copy" ), this, SLOT( handleCopyToClipboard() ) );
    menu.addAction( tr( "Paste" ), this, SLOT( handlePaste() ) );
}

void
TableView::contextMenuEvent( QContextMenuEvent * event )
{
    QMenu menu;
    addActionsToMenu( menu, event->pos() );
    menu.exec( event->globalPos() );
}
