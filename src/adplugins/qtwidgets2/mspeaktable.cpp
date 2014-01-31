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
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adportable/float.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/is_type.hpp>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QStandardItemModel>
#include <QDebug>
#include <QMenu>
#include <boost/format.hpp>

namespace qtwidgets2 {

    enum {
        c_mspeaktable_formula
        , c_mspeaktable_mass
        , c_mspeaktable_mass_error
        , c_mspeaktable_delta_mass
        , c_mspeaktable_mode
        , c_mspeaktable_time
        , c_mspeaktable_description
        , c_mspeaktable_index
        , c_mspeaktable_fcn
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
        case c_mspeaktable_mass_error:
            if ( !index.model()->data( index.model()->index( index.row(), c_mspeaktable_formula ), Qt::EditRole ).toString().isEmpty() )
                drawDisplay( painter, option, option.rect, ( boost::format("%.7g") % ( index.data( Qt::EditRole ).toDouble() * 1000 ) ).str().c_str() );
            break;
        case c_mspeaktable_mode:
        case c_mspeaktable_formula:
        case c_mspeaktable_description:
        case c_mspeaktable_num_columns:
        default:
            QItemDelegate::paint( painter, option, index );
            break;
        }
    }

    void
    MSPeakTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QItemDelegate::setModelData( editor, model, index );
        emit valueChanged( index );
    }

    std::shared_ptr< adcontrols::ChemicalFormula > MSPeakTable::formulaParser_;
}

using namespace qtwidgets2; 



MSPeakTable::MSPeakTable(QWidget *parent) : QTableView(parent)
                                          , model_( std::make_shared< QStandardItemModel >() )
										  , delegate_( std::make_shared< MSPeakTableDelegate >() )
                                          , inProgress_( false )
{
    this->setModel( model_.get() );
	this->setItemDelegate( delegate_.get() );
    this->setSortingEnabled( true );
    this->verticalHeader()->setDefaultSectionSize( 18 );
    QFont font;
    font.setFamily( "Consolas" );
	font.setPointSize( 8 );
    this->setFont( font );

    if ( ! formulaParser_ )
        formulaParser_ = std::make_shared< adcontrols::ChemicalFormula >();

    connect( this, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( showContextMenu( const QPoint& ) ) );
    connect( delegate_.get(), SIGNAL( valueChanged( const QModelIndex& ) ), this, SLOT( handleValueChanged( const QModelIndex& ) ) );
}

void *
MSPeakTable::query_interface_workaround( const char * typenam )
{
    if ( typenam == typeid( MSPeakTable ).name() )
        return static_cast< MSPeakTable * >(this);
    return 0;
}

void
MSPeakTable::OnCreate( const adportable::Configuration& )
{
}

void
MSPeakTable::OnInitialUpdate()
{
    onInitialUpdate();
}

void
MSPeakTable::onUpdate( boost::any& )
{
}

void
MSPeakTable::OnFinalClose()
{
}

bool
MSPeakTable::getContents( boost::any& ) const
{
    return false;
}

bool
MSPeakTable::setContents( boost::any& a )
{
    if ( adportable::a_type< adcontrols::MSPeakInfoPtr >::is_a( a ) ) {
        peakInfo_ = boost::any_cast< adcontrols::MSPeakInfoPtr >( a );
		if ( auto ptr = peakInfo_.lock() )
			setPeakInfo( *ptr );
    }
    return false;
}


void
MSPeakTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;
    
    model.setColumnCount( c_mspeaktable_num_columns );

    model.setHeaderData( c_mspeaktable_time,        Qt::Horizontal, QObject::tr( "time(us)" ) );
    model.setHeaderData( c_mspeaktable_mass,        Qt::Horizontal, QObject::tr( "m/z" ) );
    model.setHeaderData( c_mspeaktable_mass_error,  Qt::Horizontal, QObject::tr( "error(mDa)" ) );
    model.setHeaderData( c_mspeaktable_mode,        Qt::Horizontal, QObject::tr( "mode" ) );
    model.setHeaderData( c_mspeaktable_formula,     Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_mspeaktable_description, Qt::Horizontal, QObject::tr( "description" ) );

    setColumnHidden( c_mspeaktable_index, true );
    setColumnHidden( c_mspeaktable_fcn, true );

    resizeColumnsToContents();
    resizeRowsToContents();

    horizontalHeader()->setResizeMode( QHeaderView::Stretch );
}

void
MSPeakTable::setPeakInfo( const adcontrols::MSPeakInfo& info )
{
	QStandardItemModel& model = *model_;

    model.setRowCount( static_cast< int >( info.total_size() ) );

    adcontrols::segment_wrapper< const adcontrols::MSPeakInfo > segs( info ); // adcontrols::MSPeakInfo
	
    int row = 0;
    int fcn = 0;
    for ( auto& pkinfo: segs ) {

        int idx = 0;
        for ( auto& pk: pkinfo ) {

            model.setData( model.index( row, c_mspeaktable_fcn ), fcn ); // hidden
            model.setData( model.index( row, c_mspeaktable_index ), idx++ ); // hidden

            model.setData( model.index( row, c_mspeaktable_time ), pk.time() );
            model.setData( model.index( row, c_mspeaktable_mass ), pk.mass() );
            model.setData( model.index( row, c_mspeaktable_mode ), pkinfo.mode() );
            if ( !pk.formula().empty() ) {
                model.setData( model.index( row, c_mspeaktable_formula ), QString::fromStdString( pk.formula() ) );
                model.setData( model.index( row, c_mspeaktable_mass_error ), pk.mass() - exactMass( pk.formula() ) );
            }
			model.setData( model.index( row, c_mspeaktable_description ), QString::fromStdWString( pk.annotation() ) );

            ++row;
        }
        ++fcn;
    }
    resizeColumnsToContents();
    resizeRowsToContents();
}

void
MSPeakTable::currentChanged( const QModelIndex& index, const QModelIndex& prev )
{
    QStandardItemModel& model = *model_;
    (void)prev;

    scrollTo( index, QAbstractItemView::EnsureVisible );
	int row = index.row();
    int idx = model.index( row, c_mspeaktable_index ).data( Qt::EditRole ).toInt();
    int fcn = model.index( row, c_mspeaktable_fcn ).data( Qt::EditRole ).toInt();


    double mass = model.index( row, c_mspeaktable_mass ).data( Qt::EditRole ).toDouble();
    for ( int r = 0; r < model.rowCount(); ++r ) {
        double d = std::abs( model.index( r, c_mspeaktable_mass ).data( Qt::EditRole ).toDouble() - mass );
        model.setData( model.index( r, c_mspeaktable_delta_mass ), int( d + 0.7 ) );
    }

    emit currentChanged( idx, fcn );
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

void
MSPeakTable::showContextMenu( const QPoint& pt )
{
    std::vector< QAction * > actions;
    QMenu menu;
    
    actions.push_back( menu.addAction( "Lock mass with this peak" ) );
    QAction * selected = menu.exec( this->mapToGlobal( pt ) );
    (void)selected;
}

void
MSPeakTable::handleValueChanged( const QModelIndex& index )
{
    if ( inProgress_ )
        return;
    if ( index.column() == c_mspeaktable_formula ) {
        formulaChanged( index );
        emit valueChanged();
	}
}

void
MSPeakTable::formulaChanged( const QModelIndex& index )
{
	QStandardItemModel& model = *model_;

    if ( index.column() == c_mspeaktable_formula ) {

        std::string formula = index.data( Qt::EditRole ).toString().toStdString();

        if ( auto ptr = peakInfo_.lock() ) {
            int fcn = model.index( index.row(), c_mspeaktable_fcn ).data( Qt::EditRole ).toInt();
            int idx = model.index( index.row(), c_mspeaktable_index ).data( Qt::EditRole ).toInt();
            adcontrols::segment_wrapper< adcontrols::MSPeakInfo > segs( *ptr );
            auto it = segs[ fcn ].begin() + idx;
            it->formula( formula );
        }
        double mass = model.index( index.row(), c_mspeaktable_mass ).data( Qt::EditRole ).toDouble();
        model.setData( model.index( index.row(), c_mspeaktable_mass_error ), mass - exactMass( formula ) );
    }
}

double
MSPeakTable::exactMass( std::string formula )
{
    if ( formula.empty() )
        return 0;

    std::string adduct_lose;
    std::string::size_type pos = formula.find_first_of( "+-" );
    int sign = 1;
    if ( pos != std::wstring::npos ) {
        sign = formula.at( pos ) == '+' ? 1 : -1;
        adduct_lose = formula.substr( pos + 1 );
        formula = formula.substr( 0, pos );
    }
    
    double exactMass = formulaParser_->getMonoIsotopicMass( formula );
    if ( !adduct_lose.empty() ) {
        double a = formulaParser_->getMonoIsotopicMass( adduct_lose );
        exactMass += a * sign;
    }
    return exactMass;
}

