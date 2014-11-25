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

#include "mspeaksummary.hpp"
#include "mspeakwidget.hpp"
#include <qtwrapper/font.hpp>
#include <boost/format.hpp>
#include <QApplication>
#include <QClipboard>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QMenu>
#include <QStandardItemModel>

namespace adwidgets {
    enum {
        r_mass_vs_time
        , r_length_vs_time
        , r_result
        , r_num_rows
    };
    enum {
        c_mspeaksummary_mode
        , c_mspeaksummary_coeffs
        , c_mspeaksummary_coeffs_1
        , c_mspeaksummary_sd
        , c_mspeaksummary_t0
        , c_mspeaksummary_velocity
        , c_mspeaksummary_length
    };
    enum {
        c_mspeaksummary_formula
    };

    class MSPeakSummary::ItemDelegate : public QItemDelegate {
    public:
        explicit ItemDelegate(QObject *parent = 0) : QItemDelegate( parent ) {
        }
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            if ( index.parent() != QModelIndex() ) {
                switch ( index.column() ) {
                case c_mspeaksummary_sd:
                    drawDisplay( painter, option, option.rect, ( boost::format("%.4g") % (index.data( Qt::EditRole ).toDouble() * 1000) ).str().c_str() );
                    break;
                case c_mspeaksummary_coeffs:
                case c_mspeaksummary_coeffs_1:
                    drawDisplay( painter, option, option.rect, ( boost::format("%.8g") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
                    break;
                case c_mspeaksummary_t0:
                    drawDisplay( painter, option, option.rect, ( boost::format("%.7g") % (index.data( Qt::EditRole ).toDouble() * 1000) ).str().c_str() );
                    break;
                default:
                    QItemDelegate::paint( painter, option, index );        
                }
            } else {
                QItemDelegate::paint( painter, option, index );        
            }
        }
    };
}

using namespace adwidgets;

MSPeakSummary::MSPeakSummary(QWidget *parent) : QTreeView(parent)
                                              , model_( new QStandardItemModel )
                                              , parent_( 0 )
{
    this->setModel( model_.get() );
    this->setItemDelegate( new ItemDelegate );
    this->setSortingEnabled( true );
    QFont font;
	this->setFont( qtwrapper::font::setFont( font, qtwrapper::fontSizeSmall, qtwrapper::fontTableBody ) );
    this->setTabKeyNavigation( true );
	this->setSelectionMode( QAbstractItemView::ExtendedSelection );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QTreeView::customContextMenuRequested, this, &MSPeakSummary::showContextMenu );
}

void
MSPeakSummary::onInitialUpdate( MSPeakWidget * parent )
{
    parent_ = parent;

    QStandardItemModel& model = *model_;
    
    model.setColumnCount( 7 );
    model.setRowCount( r_num_rows );
    model.setHeaderData( 0, Qt::Horizontal, QObject::tr( "item" ) );
    model.setHeaderData( 1, Qt::Horizontal, QObject::tr( "(a)" ) );
    model.setHeaderData( 2, Qt::Horizontal, QObject::tr( "(b)" ) );
    model.setHeaderData( 3, Qt::Horizontal, QObject::tr( "std. error(ns)" ) );
    model.setHeaderData( 4, Qt::Horizontal, QObject::tr( "t0(ns)" ) );
    model.setHeaderData( 5, Qt::Horizontal, QObject::tr( "Vacc est." ) );
    model.setHeaderData( 6, Qt::Horizontal, QObject::tr( "L(m)" ) );

    model.setData( model.index( r_mass_vs_time, 0 ), "laps" );
    model.setData( model.index( r_length_vs_time, 0 ), "mol" );
    model.setData( model.index( r_result, 0 ), "normalized" );
    
    for ( int row = 0; row < r_num_rows; ++row ) {
        QStandardItem * p1 = model.itemFromIndex( model.index( row, 0 ) );
        p1->setColumnCount( 7 );
    }
}

void
MSPeakSummary::setPolynomials( int mode, const std::vector<double>& coeffs, double sd, double v, double l )
{
	QStandardItemModel& model = *model_;
    QStandardItem * parent = model.itemFromIndex( model.index( r_mass_vs_time, 0 ) );
    int row = parent->rowCount();
	for ( int r = 0; r < row; ++r ) {
		if ( model.data( model.index( r, c_mspeaksummary_mode, parent->index() ) ).toInt() == mode ) {
            row = r;
            break;
        }
	}
    if ( row >= parent->rowCount() )
        parent->setRowCount( row + 1 );
    model.setData( model.index( row, c_mspeaksummary_mode, parent->index() ), mode );
    model.setData( model.index( row, c_mspeaksummary_coeffs + 0, parent->index() ), coeffs[0] );
    model.setData( model.index( row, c_mspeaksummary_coeffs + 1, parent->index() ), coeffs[1] );
    model.setData( model.index( row, c_mspeaksummary_sd, parent->index() ), sd );

    double t0 = coeffs[0];
    model.setData( model.index( row, c_mspeaksummary_t0, parent->index() ), t0 );
    model.setData( model.index( row, c_mspeaksummary_velocity, parent->index() ), v );
    model.setData( model.index( row, c_mspeaksummary_length, parent->index() ), l );
}

void
MSPeakSummary::setPolynomials( const std::string& formula, const std::vector<double>& coeffs, double sd, double v )
{
	QStandardItemModel& model = *model_;
    QStandardItem * parent = model.itemFromIndex( model.index( r_length_vs_time, 0 ) );
    int row = parent->rowCount();
	for ( int r = 0; r < row; ++r ) {
		if ( model.data( model.index( r, c_mspeaksummary_mode, parent->index() ) ).toString() == QString::fromStdString(formula) ) {
            row = r;
            break;
        }
	}
    if ( row >= parent->rowCount() )
        parent->setRowCount( row + 1 );

    model.setData( model.index( row, c_mspeaksummary_mode, parent->index() ), QString::fromStdString( formula ) );
    model.setData( model.index( row, c_mspeaksummary_coeffs + 0, parent->index() ), coeffs[0] );
    model.setData( model.index( row, c_mspeaksummary_coeffs + 1, parent->index() ), coeffs[1] );
    model.setData( model.index( row, c_mspeaksummary_sd, parent->index() ), sd );
    double t0 = coeffs[0];
    model.setData( model.index( row, c_mspeaksummary_t0, parent->index() ), t0 );
    model.setData( model.index( row, c_mspeaksummary_velocity, parent->index() ), v );
}

void
MSPeakSummary::setResult( int row, const std::vector<double>& coeffs, double sd )
{
	QStandardItemModel& model = *model_;
    QStandardItem * parent = model.itemFromIndex( model.index( r_result, 0 ) );

    if ( row >= parent->rowCount() )
        parent->setRowCount( row + 1 );

    model.setData( model.index( row, c_mspeaksummary_mode, parent->index() ), row );
    model.setData( model.index( row, c_mspeaksummary_coeffs + 0, parent->index() ), coeffs[0] );
    model.setData( model.index( row, c_mspeaksummary_coeffs + 1, parent->index() ), coeffs[1] );
    model.setData( model.index( row, c_mspeaksummary_sd, parent->index() ), sd );
    // model.setData( model.index( row, c_mspeaksummary_t0, parent->index() ), t0 );
}

void
MSPeakSummary::clear()
{
    QStandardItemModel& model = *model_;

    for ( int row = 0; row < r_num_rows; ++row ) {
        QStandardItem * p1 = model.itemFromIndex( model.index( row, 0 ) );
        p1->setRowCount( 0 );
    }
}

// override QTreeView
void
MSPeakSummary::currentChanged( const QModelIndex& index, const QModelIndex& prev )
{
    QStandardItemModel& model = *model_;
    (void)prev;

    scrollTo( index, QAbstractItemView::EnsureVisible );
    if ( index.parent() != QModelIndex() ) {
        int which = index.parent().row();
        int row = index.row();
        if ( which == 0 ) {
            int mode = model.index( row, 0, index.parent() ).data( Qt::EditRole ).toInt();
			if ( parent_ )
				parent_->currentChanged( mode );
        } else if ( which == 1 ) {
            std::string formula = model.index( row, 0, index.parent() ).data( Qt::EditRole ).toString().toStdString();
			if ( parent_ )
				parent_->currentChanged( formula );
        }
    }
}

void
MSPeakSummary::keyPressEvent( QKeyEvent * event )
{
    if ( event->matches( QKeySequence::Copy ) ) {
        handleCopyToClipboard();
    } else if ( event->matches( QKeySequence::Paste ) ) {
        // handlePasteFromClipboard();
    } else {
        QTreeView::keyPressEvent( event );
    }
}

void
MSPeakSummary::handleCopyToClipboard()
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
        copy_table.append( model.data( idx, Qt::DisplayRole ).toString() );
        prev = idx;
    }
    QApplication::clipboard()->setText( copy_table );
}

void
MSPeakSummary::handleClear()
{
    emit onClear();
}

void
MSPeakSummary::showContextMenu( const QPoint& pt )
{
    QMenu menu;
    
    menu.addAction( "Copy", this, SLOT( handleCopyToClipboard() ) );
    menu.addAction( "Clear", this, SLOT( handleClear() ) );

    menu.exec( this->mapToGlobal( pt ) );
}
