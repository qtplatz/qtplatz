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

#include "msquantable.hpp"
#include "htmlheaderview.hpp"
#include "delegatehelper.hpp"
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msqpeak.hpp>
#include <adcontrols/msqpeaks.hpp>
#include <adportable/float.hpp>
#include <qtwrapper/font.hpp>

#include <QApplication>
#include <QBrush>
#include <QClipboard>
//#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QKeyEvent>

#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <functional>

namespace adwidgets {
    namespace detail {

        enum {
            c_datasource
            , c_component   // this is the primary id of component. if same key is assined to more than two formula, quant. result will be summed
            , c_formula      // 
            , c_mass
            , c_delta_mass
            , c_intensity
            , c_relative_intensity
            , c_mode
            , c_time
            , c_protocol
            , c_description
            , c_amount
            , c_isSTD
            , c_istd
            , c_idx
            , c_fcn
            , c_id // data guid 
            , c_num_columns
        };
        using namespace adcontrols::metric;
        
        class ItemDelegate : public QStyledItemDelegate { //QItemDelegate {
        public:
            explicit ItemDelegate( QObject *parent = 0 ) : QStyledItemDelegate( parent ) {
            }

            void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {

                QStyleOptionViewItem op( option );
                auto align = Qt::AlignRight | Qt::AlignHCenter;
                op.displayAlignment = Qt::AlignRight | Qt::AlignHCenter;

                // if ( index.row() & 0x01 )
                //     op.palette.setColor( QPalette::Text, Qt::green );
                painter->save();
                const QAbstractItemModel& model = *index.model();

                QString dataId = model.data( model.index( currIndex_.row(), c_id ) ).toString();
                if ( model.data( model.index( index.row(), c_id ) ).toString() == dataId ) {
                    painter->setPen( Qt::blue );
                    painter->fillRect( op.rect, QColor( 192, 192, 128, 0x80 ) );
                }

                switch( index.column() ) {
                case c_datasource:
                    do {
                        std::wstring fqn = index.data( Qt::EditRole ).toString().toStdWString();
                        std::size_t pos = fqn.find( L"::" );
                        if ( pos != std::wstring::npos ) {
                            boost::filesystem::path path( fqn.substr( 0, pos ) );
                            painter->drawText( op.rect, op.displayAlignment, QString::fromStdWString( path.stem().wstring() + L"/" + fqn.substr( pos + 2 ) ) );
                            //drawDisplay( painter, option, option.rect, QString::fromStdWString( path.stem().wstring() + L"/" + fqn.substr( pos + 2 ) ) );
                            
                        } else 
                            painter->drawText( op.rect, align, QString::fromStdWString( fqn ) );
                    } while ( 0 );
                    break;
                case c_time:
                    painter->drawText( option.rect, align, (boost::format( "%.5lf" ) % scale_to_micro( index.data( Qt::EditRole ).toDouble() )).str().c_str() );
                    break;
                case c_mass:
                    painter->drawText( option.rect, align, (boost::format( "%.7lf" ) % index.data( Qt::EditRole ).toDouble()).str().c_str() );
                    break;
                case c_intensity:
                case c_relative_intensity:
                    if ( !index.model()->data( index.model()->index( index.row(), c_intensity ), Qt::EditRole ).toString().isEmpty() )
                        painter->drawText( op.rect, align, (boost::format( "%.2lf" ) % (index.data( Qt::EditRole ).toDouble())).str().c_str() );
                    break;
                default:
                    op.displayAlignment = Qt::AlignCenter | Qt::AlignHCenter;
                    QStyledItemDelegate::paint( painter, op, index );
                }
                painter->restore();
            }

            void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
                QStyledItemDelegate::setModelData( editor, model, index );
                if ( valueChanged_ )
                    valueChanged_( index );
            }

            bool editorEvent( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem& option, const QModelIndex& index ) override {
                bool res = QStyledItemDelegate::editorEvent( event, model, option, index );
                if ( event->type() == QEvent::MouseButtonRelease && model->flags(index) & Qt::ItemIsUserCheckable ) {
                    QVariant st = index.data( Qt::CheckStateRole );
                    if ( index.column() == c_isSTD )
                        model->setData( index, (st == Qt::Checked) ? "STD" : "SAMP" );
                    if ( valueChanged_ )
                        valueChanged_( index );
                }
                return res;
            }
            
            std::function<void( const QModelIndex& )> valueChanged_;
            QModelIndex currIndex_;
        public:
            void handleCurrentChanged( const QModelIndex& index ) {
                currIndex_ = index;
            }
        };
    }
}

using namespace adwidgets;
using namespace adwidgets::detail;

MSQuanTable::MSQuanTable( QWidget * parent ) : QTableView( parent )
                                                 , model_( new QStandardItemModel )
                                                 , inProgress_( false )
{
    setHorizontalHeader( new HtmlHeaderView );
    setModel( model_ );
    if ( auto delegate = new detail::ItemDelegate() ) {
        delegate->valueChanged_ = [=] ( const QModelIndex& idx ){ handleValueChanged( idx ); };
        setItemDelegate( delegate );
        connect( this, static_cast< void (MSQuanTable::*)(const QModelIndex&)>(&MSQuanTable::currentChanged), delegate, &ItemDelegate::handleCurrentChanged );
    }
    setSortingEnabled( true );
    verticalHeader()->setDefaultSectionSize( 18 );
    setContextMenuPolicy( Qt::CustomContextMenu );

    QFont font;
    this->setFont( qtwrapper::font::setFont( font, qtwrapper::fontSizeSmall, qtwrapper::fontTableBody ) );

    connect( this, &QTableView::customContextMenuRequested, this, &MSQuanTable::handleContextMenuRequested );
}

void
MSQuanTable::OnCreate( const adportable::Configuration& )
{
}

void
MSQuanTable::OnInitialUpdate()
{
    QStandardItemModel & model = *model_;
    
    model.setColumnCount( c_num_columns );

    model.setHeaderData( c_datasource,  Qt::Horizontal, QObject::tr( "data source" ) );
    model.setHeaderData( c_component,   Qt::Horizontal, QObject::tr( "component" ) );
    model.setHeaderData( c_formula,     Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_mass,        Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
    model.setHeaderData( c_delta_mass,  Qt::Horizontal, QObject::tr( "&delta;Da" ) );
    model.setHeaderData( c_intensity,   Qt::Horizontal, QObject::tr( "Abandance" ) );
    model.setHeaderData( c_relative_intensity,   Qt::Horizontal, QObject::tr( "R.A." ) );
    model.setHeaderData( c_mode,        Qt::Horizontal, QObject::tr( "mode" ) );
    model.setHeaderData( c_time,        Qt::Horizontal, QObject::tr( "time(&mu;s)" ) );
    model.setHeaderData( c_protocol,    Qt::Horizontal, QObject::tr( "protocol" ) );
    model.setHeaderData( c_description, Qt::Horizontal, QObject::tr( "description" ) );
    model.setHeaderData( c_amount,      Qt::Horizontal, QObject::tr( "Amount" ) );
    model.setHeaderData( c_isSTD,       Qt::Horizontal, QObject::tr( "Std." ) );    
    model.setHeaderData( c_istd,        Qt::Horizontal, QObject::tr( "I.S.#" ) );
    model.setHeaderData( c_id,          Qt::Horizontal, QObject::tr( "Data ID" ) );
    model.setHeaderData( c_idx,         Qt::Horizontal, QObject::tr( "idx#" ) );
    model.setHeaderData( c_fcn,         Qt::Horizontal, QObject::tr( "fcn#" ) );

    //setColumnHidden( c_idx, true );
    //setColumnHidden( c_fcn, true );  // a.k.a. protocol id, internally used as an id
    //setColumnHidden( c_id, true );
}

void
MSQuanTable::handleSelected( const QRectF& rc, bool isTime )
{
    QStandardItemModel& model = *model_;
    
    double y0 = 0;
    int row_highest = -1;
	for ( int row = 0; row < model.rowCount(); ++row ) {
        QModelIndex index = model.index( row, isTime ? c_time : c_mass );
        double t = index.data( Qt::EditRole ).toDouble();
        if ( rc.left() < t && t < rc.right() ) {
            double y = model.index( row, c_intensity ).data( Qt::EditRole ).toDouble();
            if ( y > y0 ) {
                y0 = y;
                row_highest = row;
            }
        }
    }
    if ( row_highest >= 0 ) {
		setCurrentIndex( model.index( row_highest, c_formula ) );
		scrollTo( model.index( row_highest, c_formula ) );
    }
}

void
MSQuanTable::onUpdate( boost::any& )
{
}

void
MSQuanTable::OnFinalClose()
{
}

bool
MSQuanTable::getContents( boost::any& ) const
{
    return false;
}

bool
MSQuanTable::setContents( boost::any& )
{
    return false;
}

void *
MSQuanTable::query_interface_workaround( const char * typnam )
{
    if ( typnam == typeid( MSQuanTable ).name() )
        return this;
    return 0;
}

// reimplement QTableView
void
MSQuanTable::currentChanged( const QModelIndex& index, const QModelIndex& )
{
    QStandardItemModel& model = *model_;

    scrollTo( index, QAbstractItemView::EnsureVisible );
	int row = index.row();
    int idx = model.index( row, c_idx ).data( Qt::EditRole ).toInt();
    int fcn = model.index( row, c_fcn ).data( Qt::EditRole ).toInt();
    std::wstring dataGuid = model.index( row, c_id ).data( Qt::EditRole ).toString().toStdWString();
    QString profGuid;
    if ( auto pks = qpks_.lock() )
        profGuid = QString::fromStdWString( pks->parentGuid( dataGuid ) );

    setUpdatesEnabled( false );
    double mass = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
    for ( int r = 0; r < model.rowCount(); ++r ) {
        double d = std::abs( model.index( r, c_mass ).data( Qt::EditRole ).toDouble() - mass );
        model.setData( model.index( r, c_delta_mass ), int( d + 0.7 ) );
    }
    setUpdatesEnabled( true );
    emit currentChanged( idx, fcn, QString::fromStdWString( dataGuid ), profGuid );
    emit currentChanged( index );
}

void
MSQuanTable::keyPressEvent( QKeyEvent * event )
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
MSQuanTable::valueChanged()
{
}

void
MSQuanTable::currentChanged( int idx, int fcn )
{
}

void
MSQuanTable::formulaChanged( int idx, int fcn )
{
}

void
MSQuanTable::handleCopyToClipboard()
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
        if ( idx.column() == c_time )
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
MSQuanTable::handle_zoomed( const QRectF& )   // zoomer zoomed
{
}

void
MSQuanTable::handleValueChanged( const QModelIndex& index )
{
    if ( inProgress_ )
        return;

    QStandardItemModel& model = *model_;

    if ( auto pks = qpks_.lock() ) {
        int fcn = model.index( index.row(), c_fcn ).data().toInt();
        int idx = model.index( index.row(), c_idx ).data().toInt();
        std::wstring dataGuid = model.index( index.row(), c_id ).data().toString().toStdWString();

        auto it = std::find_if( pks->begin(), pks->end(), [=] ( adcontrols::MSQPeaks::value_type& t ){
                return t.dataGuid() == dataGuid && t.fcn() == fcn && t.idx() == idx; } );
        if ( it != pks->end() ) {
            if ( index.column() == c_isSTD ) {
                bool standard = (index.data( Qt::CheckStateRole ) == Qt::Checked);
                it->isSTD( standard );
                if ( standard && adportable::compare<double>::essentiallyEqual( it->amount(), 0 ) ) {
                    it->amount( 1.0 );
                    model.setData( model.index( index.row(), c_amount ), it->amount() );
                }
            }
            if ( index.column() == c_formula ) {
                it->formula( index.data().toString().toStdString() );
            }
            else if ( index.column() == c_component ) {
                it->componentId( index.data().toString().toStdWString() );
            }
            else if ( index.column() == c_description ) {
                it->description( index.data().toString().toStdString() );
            }
        }
    }
}

void
MSQuanTable::handleContextMenuRequested( const QPoint& )
{
}


void
MSQuanTable::setData( const adcontrols::MSQPeaks * pks )
{
    if ( pks )
        qpks_ = pks->shared_from_this();

	QStandardItemModel& model = *model_;

    setUpdatesEnabled( false );
    // model.blockSignals( true );

    std::map< std::wstring, double > sum;

    if ( pks->size() != model.rowCount() )
        model.setRowCount( int( pks->size() ) );

    int row = 0;
    for ( auto& pk: *pks ) {
        model.setData( model.index( row, c_fcn ), pk.fcn() );
        model.setData( model.index( row, c_idx ), pk.idx() );
        model.setData( model.index( row, c_id ),  QString::fromStdWString( pk.dataGuid()) );

        if ( sum.find( pk.dataGuid() ) == sum.end() ) {
            sum[ pk.dataGuid() ] = 0;
            std::for_each( pks->begin(), pks->end(), [&]( const adcontrols::MSQPeak& t ){
                    if ( pk.dataGuid() == t.dataGuid() )
                        sum[ pk.dataGuid() ] += t.intensity();
                });
        }

        model.setData( model.index( row, c_datasource ),  QString::fromStdWString( pks->dataSource( pk.dataGuid() ) ) );
        model.setData( model.index( row, c_protocol ), QString::fromStdString( pk.protocol() ) );
        model.setData( model.index( row, c_amount ), pk.amount() );
        model.setData( model.index( row, c_component ), QString::fromStdWString( pk.componentId() ) );

        double ra = pk.intensity() * 100 / sum[ pk.dataGuid() ];
        model.setData( model.index( row, c_relative_intensity ), ra );

        model.setData( model.index( row, c_time ), pk.time() );
        model.setData( model.index( row, c_mass ), pk.mass() );
        model.setData( model.index( row, c_intensity ), pk.intensity() );
        model.setData( model.index( row, c_mode ), pk.mode() );
        model.setData( model.index( row, c_formula ), QString::fromStdString( pk.formula() ) );
        model.setData( model.index( row, c_description ), QString::fromStdString( pk.description() ) );
        if ( auto chk = model.itemFromIndex( model.index( row, c_istd ) ) ) {
            // internal standard, IS# 
            chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | chk->flags() );
            model.setData( model.index( row, c_istd ), pk.isIS() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
            // set IS#
            model.setData( model.index( row, c_istd ), pk.istd() );
        }
        if ( auto chk = model.itemFromIndex( model.index( row, c_isSTD ) ) ) {
            // standard | sample
            chk->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | chk->flags() );
            model.setData( model.index( row, c_isSTD ), pk.isSTD() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
        ++row;
    }

    // model.blockSignals( false );
    setUpdatesEnabled( true );
    // update();
}
