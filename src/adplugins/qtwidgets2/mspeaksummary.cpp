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

#include "mspeaksummary.hpp"
#include "mspeakview.hpp"
#include <QStandardItemModel>
#include <boost/format.hpp>

namespace qtwidgets2 {
    enum {
        r_mass_vs_time
        , r_length_vs_time
        , r_num_rows
    };
    enum {
        c_mspeaksummary_mode
        , c_mspeaksummary_coeffs
        , c_mspeaksummary_coeffs_1
        , c_mspeaksummary_sd
        , c_mspeaksummary_t0
    };
    enum {
        c_mspeaksummary_formula
    };
}

using namespace qtwidgets2;

MSPeakSummary::MSPeakSummary(QWidget *parent) : QTreeView(parent)
                                              , model_( new QStandardItemModel )
                                              , delegate_( new MSPeakSummaryDelegate )
                                              , parent_( 0 )
{
    this->setModel( model_.get() );
    this->setItemDelegate( delegate_.get() );
    this->setSortingEnabled( true );
    //this->verticalHeader()->setDefaultSectionSize( 18 );
    QFont font;
    font.setFamily( "Consolas" );
	font.setPointSize( 8 );
    this->setFont( font );
    this->setTabKeyNavigation( true );
	// this->setEditTriggers( QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked );
}

void
MSPeakSummary::onInitialUpdate( MSPeakView * parent )
{
    parent_ = parent;

    QStandardItemModel& model = *model_;
    
    model.setColumnCount( 5 );
    model.setRowCount( r_num_rows );
    model.setHeaderData( 0, Qt::Horizontal, QObject::tr( "item" ) );
    model.setHeaderData( 1, Qt::Horizontal, QObject::tr( "(a)" ) );
    model.setHeaderData( 2, Qt::Horizontal, QObject::tr( "(b)" ) );
    model.setHeaderData( 3, Qt::Horizontal, QObject::tr( "std. error" ) );
    model.setHeaderData( 4, Qt::Horizontal, QObject::tr( "t0 (us)" ) );

    model.setData( model.index( r_mass_vs_time, 0 ), "#lap" );
    model.setData( model.index( r_length_vs_time, 0 ), "mol" );
    
    QStandardItem * p1 = model.itemFromIndex( model.index( r_mass_vs_time, 0 ) );
	p1->setColumnCount( 5 );

    QStandardItem * p2 = model.itemFromIndex( model.index( r_length_vs_time, 0 ) );
	p2->setColumnCount( 5 );
}

void
MSPeakSummary::setPolinomials( int mode, const std::vector<double>& coeffs, double sd )
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

    double t0 = (-coeffs[0]) / coeffs[1];
    model.setData( model.index( row, c_mspeaksummary_t0, parent->index() ), t0 );
}

void
MSPeakSummary::setPolinomials( const std::string& formula, const std::vector<double>& coeffs, double sd )
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
    double t0 = (-coeffs[0]) / coeffs[1];
    model.setData( model.index( row, c_mspeaksummary_t0, parent->index() ), t0 );
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

////////////////

MSPeakSummaryDelegate::MSPeakSummaryDelegate(QObject *parent) : QItemDelegate( parent )
{
}
        
void
MSPeakSummaryDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.parent() != QModelIndex() ) {
        switch ( index.column() ) {
        case c_mspeaksummary_sd:
            drawDisplay( painter, option, option.rect, ( boost::format("%.4le") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
            break;
        case c_mspeaksummary_coeffs:
        case c_mspeaksummary_coeffs_1:
            drawDisplay( painter, option, option.rect, ( boost::format("%.7le") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
            break;
        default:
            QItemDelegate::paint( painter, option, index );        
        }
    } else {
        QItemDelegate::paint( painter, option, index );        
    }
}

