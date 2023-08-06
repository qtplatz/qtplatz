/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adportable/algorithm.hpp>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QList>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QDebug>
#include <QRegularExpression>
#include <set>

namespace {

    struct HasUserCheckableItem {
        bool operator()( QModelIndexList&& indices ) const {
            for ( const auto& index: indices ) {
                auto flags = index.model()->flags( index );
                if ( flags & (Qt::ItemIsUserCheckable | Qt::ItemIsEnabled ) )
                    return true;
            }
            return false;
        }

        bool operator()( const QAbstractItemModel * model ) const {
            for ( int row = 0; row < model->rowCount(); ++row ) {
                for ( int col = 0; col < model->columnCount(); ++col ) {
                    auto flags = model->flags( model->index( row, col ) );
                    if ( flags & (Qt::ItemIsUserCheckable | Qt::ItemIsEnabled ) )
                        return true;
                }
            }
            return false;
        }
    };

    struct CheckStateChange {
        CheckStateChange( QAbstractItemModel * model, QModelIndexList&& indices )
            : model_( model )
            , indices_( std::move( indices ) ) {}

        void operator()( Qt::CheckState state ) const {
            for ( auto& index: indices_ ){
                auto flags = index.model()->flags( index );
                if ( flags & Qt::ItemIsUserCheckable ) {
                    model_->setData( index, state, Qt::CheckStateRole );
                }
            }
        }

        void toggle() const {
            for ( auto& index: indices_ ){
                auto flags = index.model()->flags( index );
                if ( flags & Qt::ItemIsUserCheckable ) {
                    Qt::CheckState state = Qt::CheckState( index.data( Qt::CheckStateRole ).toUInt() );
                    model_->setData( index, state == Qt::Checked ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole );
                }
            }
        }
        QAbstractItemModel * model_;
        QModelIndexList indices_;

    };

}

using namespace adwidgets;
using adportable::equal_range;

TableView::TableView(QWidget *parent) : QTableView(parent)
                                      , allowDelete_( true )
{
    verticalHeader()->setDefaultSectionSize( 18 );
    verticalHeader()->setFixedWidth( 24 );
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
    } else if ( event->matches( QKeySequence::InsertLineSeparator ) ) { // mac:Meta+Enter|Meta+O otherwise Shift+Enter
        handleInsertLine();
	} else
		QTableView::keyPressEvent( event );
}

void
TableView::mouseReleaseEvent( QMouseEvent * event )
{
	QModelIndex index = indexAt( event->pos() );

    QTableView::mouseReleaseEvent( event );

	if ( index.isValid() ) {
        Qt::ItemFlags flags = model()->flags( index );
        if ( flags & Qt::ItemIsUserCheckable ) {
            QVariant st = index.data( Qt::CheckStateRole );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			if ( index.data( Qt::EditRole ).type() == QVariant::Bool )
                model()->setData( index, ( st == Qt::Checked ) ? true : false );
#else
			if ( index.data( Qt::EditRole ).metaType() == QMetaType::fromType< bool >() )
                model()->setData( index, ( st == Qt::Checked ) ? true : false );
#endif
        }
    }
}

void
TableView::handlePaste()
{
}

void
TableView::handleInsertLine()
{
    auto index = selectionModel()->currentIndex();
    if ( index.isValid() ) {
        if ( auto model = this->model() ) {
            model->insertRow( index.row() + 1, index.parent() );
            emit lineInserted( model->index( index.row() + 1, index.column(), index.parent() ) );
        }
    }
}

namespace {

    QString __remove_html( QString&& s, bool enable ) {
        return enable == false ? s.remove( QRegularExpression( "<[^>]*>" ) ) : s;
    }
}

void
TableView::handleCopyToClipboard()
{
    bool keyShift = bool( QApplication::keyboardModifiers() & Qt::ShiftModifier );
    copyToClipboard( keyShift );
}

void
TableView::copyToClipboard( bool enable_html )
{
	QModelIndexList indices = selectionModel()->selectedIndexes();

    std::sort( indices.begin(), indices.end() );
    if ( indices.size() < 1 )
        return;

    QString selected_text;

    // copy header --->
    std::set< int > hCols;
    for ( const auto& index: indices )
        hCols.insert( index.column() );

    for ( int col: hCols ) {
        if ( !isColumnHidden( col ) ) {
            selected_text.append( __remove_html( model()->headerData( col, Qt::Horizontal ).toString(), enable_html ) );
            selected_text.append( '\t' );
        }
    }
    selected_text.append( '\n' );
    // <-------------
    std::pair< QModelIndexList::const_iterator, QModelIndexList::const_iterator > range{ indices.begin(), {} };
    while ( range.first != indices.end() ) {
        range = equal_range( indices.begin(), indices.end(), *range.first
                             , [](const auto& a, const auto& b){ return a.row() < b.row(); });
        // per line
        for ( auto it = range.first; it != range.second; ++it ) {
            if ( ! isColumnHidden( it->column() ) ) {
                selected_text.append( __remove_html( it->data( Qt::EditRole ).toString(), enable_html ) );
                selected_text.append( '\t' );
            }
        }
        selected_text.append( '\n' );
        range.first = range.second;
    }
    QMimeData * md = new QMimeData();
    md->setText( selected_text );
    QApplication::clipboard()->setMimeData( md );
}

void
TableView::handleDeleteSelection()
{
	QModelIndexList indices = selectionModel()->selectedIndexes();

    if ( indices.empty() )
        return;

	std::set< int > rows;
    std::for_each( indices.begin(), indices.end(), [&](const auto& index){ rows.insert( index.row() ); } );

    emit rowsAboutToBeRemoved( rows );

	std::vector< std::pair< int, int > > ranges;

	for ( auto it = rows.begin(); it != rows.end(); ++it ) {
		std::pair< int, int > range{*it, *it};
		while ( ++it != rows.end() && *it == std::get< 1 >( range ) + 1 )
            std::get< 1 >( range ) = *it;
		--it;
		ranges.emplace_back( range );
	}

	// remove from botton to top
	for ( auto range = ranges.rbegin(); range != ranges.rend(); ++range )
		model()->removeRows( range->first, range->second - range->first + 1 );

    emit rowsDeleted();
}

void
TableView::addActionsToContextMenu( QMenu& menu, const QPoint& ) const
{
    if ( HasUserCheckableItem()( model() ) ) {
        CheckStateChange checkStateChange( model(), selectionModel()->selectedIndexes() );
        menu.addAction( tr( "Toggle checked" ),    [=](){ checkStateChange.toggle(); } );
        menu.addAction( tr( "Check selected" ),    [=](){ checkStateChange( Qt::Checked ); } );
        menu.addAction( tr( "Uncheck selected" ),  [=](){ checkStateChange( Qt::Unchecked ); } );
    }

    menu.addAction( tr( "Copy" ), this, SLOT( handleCopyToClipboard() ) );
    menu.addAction( tr( "Paste" ), this, SLOT( handlePaste() ) );
    menu.addAction( tr( "Delete" ), this, SLOT( handleDeleteSelection() ) )->setEnabled( allowDelete_ );
    menu.addAction( tr( "Insert row (Shift+Enter)" ), this, SLOT( handleInsertLine() ) )->setEnabled( allowDelete_ );
}

void
TableView::contextMenuEvent( QContextMenuEvent * event )
{
    QMenu menu;
    addActionsToContextMenu( menu, event->pos() );
    menu.exec( event->globalPos() );
}
