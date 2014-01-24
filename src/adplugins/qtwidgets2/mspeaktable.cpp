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

#include "mspeaktable.hpp"
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adportable/float.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QStandardItemModel>
#include <boost/format.hpp>

namespace qtwidgets2 {

    enum {
        c_mspeaktable_formula
        , c_mspeaktable_time
        , c_mspeaktable_mass
        , c_mspeaktable_mode
        , c_mspeaktable_flength
        , c_mspeaktable_accelerator_voltage
        , c_mspeaktable_description
        , c_mspeaktable_spectrumId
        , c_mspeaktable_num_columns
    };

	using namespace adcontrols::metric;

    MSPeakTableDelegate::MSPeakTableDelegate(QObject *parent) : QItemDelegate( parent )
    {
    }
        
    void
    MSPeakTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        switch( index.column() ) {
        case c_mspeaktable_time:
            drawDisplay( painter, option, option.rect
                         , ( boost::format("%.5lf") % scale_to_micro( index.data( Qt::EditRole ).toDouble() ) ).str().c_str() );
            break;
        case c_mspeaktable_mass:
            drawDisplay( painter, option, option.rect, ( boost::format("%.7lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
            break;
		case c_mspeaktable_flength:
            drawDisplay( painter, option, option.rect, ( boost::format("%.6lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
            break;
        case c_mspeaktable_accelerator_voltage:
            drawDisplay( painter, option, option.rect, ( boost::format("%.1lf") % index.data( Qt::EditRole ).toDouble() ).str().c_str() );
            break;
        case c_mspeaktable_mode:
        case c_mspeaktable_formula:
        case c_mspeaktable_description:
        case c_mspeaktable_spectrumId:
        case c_mspeaktable_num_columns:
        default:
            QItemDelegate::paint( painter, option, index );
            break;
        }
    }
}

using namespace qtwidgets2; 


MSPeakTable::MSPeakTable(QWidget *parent) : QTableView(parent)
                                          , model_( std::make_shared< QStandardItemModel >() )
										  , delegate_( std::make_shared< MSPeakTableDelegate >() )
{
    this->setModel( model_.get() );
	this->setItemDelegate( delegate_.get() );
    this->setSortingEnabled( true );
    this->verticalHeader()->setDefaultSectionSize( 18 );
    QFont font;
    font.setFamily( "Consolas" );
	font.setPointSize( 8 );
    this->setFont( font );
}

void
MSPeakTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;
    
    model.setColumnCount( c_mspeaktable_num_columns );
    model.setHeaderData( c_mspeaktable_time, Qt::Horizontal, QObject::tr( "time(us)" ) );
    model.setHeaderData( c_mspeaktable_mass, Qt::Horizontal, QObject::tr( "m/z" ) );
    model.setHeaderData( c_mspeaktable_mode, Qt::Horizontal, QObject::tr( "lap#" ) );
    model.setHeaderData( c_mspeaktable_formula, Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_mspeaktable_description, Qt::Horizontal, QObject::tr( "description" ) );
    model.setHeaderData( c_mspeaktable_spectrumId, Qt::Horizontal, QObject::tr( "spectrum id" ) );
    model.setHeaderData( c_mspeaktable_accelerator_voltage, Qt::Horizontal, QObject::tr( "V(acc)" ) );

    resizeRowsToContents();
    resizeColumnsToContents();
    horizontalHeader()->setResizeMode( QHeaderView::Stretch );
}

void
MSPeakTable::setPeaks( const adcontrols::MSPeaks& peaks )
{
    model_->setRowCount(0);
    for ( auto& peak: peaks )
        addPeak( peak );

    resizeRowsToContents();
    resizeColumnsToContents();
}

void
MSPeakTable::addPeak( const adcontrols::MSPeak& peak )
{
	QStandardItemModel& model = *model_;

	int row = model.rowCount();
	model.setRowCount( row + 1 );

	model.setData( model.index( row, c_mspeaktable_time ), peak.time() );
	model.setData( model.index( row, c_mspeaktable_mass ), peak.mass() );
	model.setData( model.index( row, c_mspeaktable_mode ), peak.mode() );
	model.setData( model.index( row, c_mspeaktable_flength ), peak.flight_length() );
	model.setData( model.index( row, c_mspeaktable_formula ), QString::fromStdString( peak.formula() ) );
	model.setData( model.index( row, c_mspeaktable_description ), QString::fromStdWString( peak.description() ) );
    model.setData( model.index( row, c_mspeaktable_spectrumId ),  QString::fromStdString( peak.spectrumId() ) );
    double vacc = adportable::TimeSquaredScanLaw::acceleratorVoltage( peak.mass(), peak.time(), peak.flight_length(), 0.0 );
    model.setData( model.index( row, c_mspeaktable_accelerator_voltage ), vacc );
}

void
MSPeakTable::keyPressEvent( QKeyEvent * event )
{
    if ( event->matches( QKeySequence::Copy ) ) {
        handleCopyToClipboard();
    } else if ( event->matches( QKeySequence::Paste ) ) {
        // handlePasteFromClipboard();
    } else {
        QTableView::keyPressEvent( event );
    }
}

void
MSPeakTable::handleCopyToClipboard()
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
        if ( idx.column() == c_mspeaktable_time )
            copy_table.append( (boost::format("%.14g") % adcontrols::metric::scale_to_micro( model.data( idx ).toDouble() )).str().c_str() );
		else if ( model.data( idx ).type() == QVariant::Double )
			copy_table.append( (boost::format("%.14g") % model.data( idx ).toDouble()).str().c_str() );
        else
            copy_table.append( model.data( idx ).toString() );
        prev = idx;
    }
    QApplication::clipboard()->setText( copy_table );
}
