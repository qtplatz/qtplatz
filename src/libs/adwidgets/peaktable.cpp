/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "peaktable.hpp"
#include "htmlheaderview.hpp"
#include "delegatehelper.hpp"
#include <QStandardItemModel>
#include <QItemDelegate>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <qtwrapper/qstring.hpp>
#include <boost/format.hpp>
#include <functional>

namespace adwidgets {
    namespace peaktable {

        enum {
            c_id
            , c_name
            , c_tr
            , c_area
            , c_height
            , c_width
            , c_ntp
            , c_rs
            , c_asymmetry
            , c_capacityfactor
            , nbr_of_columns
        };

        using namespace adcontrols::metric;

        class ItemDelegate : public QItemDelegate {
        public:
            explicit ItemDelegate( QObject *parent = 0 ) : QItemDelegate( parent ) {
            }

            void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {

                if ( !index.isValid() )
                    return;

                QStyleOptionViewItem op( option );

                if ( index.column() > c_name )
                    op.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

                switch( index.column() ) {
                case c_tr:
                    drawDisplay( painter, op, op.rect, QString::number( index.data().toDouble(), 'f', 4 ) );
                    break;
                case c_name:
                    DelegateHelper::render_html2( painter, option, index.data().toString() );
                    break;
                case c_area:
                    drawDisplay( painter, op, op.rect, QString::number( index.data().toDouble(), 'e', 5 ) );
                    break;
                case c_height:
                    drawDisplay( painter, op, op.rect, QString::number( index.data().toDouble(), 'f', 1 ) );
                    break;
                case c_width:
                    drawDisplay( painter, op, op.rect, QString::number( index.data().toDouble(), 'f', 3 ) );
                    break;
                case c_ntp:
                    drawDisplay( painter, op, op.rect, QString::number( index.data().toDouble(), 'f', 1 ) );
                    break;
                case c_rs:
                case c_asymmetry:
                case c_capacityfactor:
                    drawDisplay( painter, op, op.rect, QString::number( index.data().toDouble(), 'f', 2 ) );
                    break;
                default:
                    QItemDelegate::paint( painter, op, index );
                    break;
                }
            }

            void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
                QItemDelegate::setModelData( editor, model, index );
                if ( valueChanged_ )
                    valueChanged_( index );
            }

            std::function<void( const QModelIndex& )> valueChanged_;
        };

    }
}

using namespace adwidgets;
using namespace adwidgets::peaktable;

PeakTable::~PeakTable()
{
    delete model_;
}

PeakTable::PeakTable( QWidget *parent ) : TableView( parent )
                                        , model_( new QStandardItemModel )
{
    setHorizontalHeader( new HtmlHeaderView( Qt::Horizontal, this ) );
    setModel( model_ );

    if ( auto delegate = new peaktable::ItemDelegate() ) {
        delegate->valueChanged_ = [=] ( const QModelIndex& idx ){ emit valueChanged( idx.row() ); };
        setItemDelegate( delegate );
    }

    setSortingEnabled( true );
}

void
PeakTable::OnCreate( const adportable::Configuration& )
{
}

void
PeakTable::OnInitialUpdate()
{
    QStandardItemModel& model = *model_;

    QStandardItem * rootNode = model.invisibleRootItem();
    setRowHeight( 0, 7 );
    rootNode->setColumnCount( nbr_of_columns );

    model.setHeaderData( c_id, Qt::Horizontal, QObject::tr("Id") );
    model.setHeaderData( c_name, Qt::Horizontal, QObject::tr("Name") );
    model.setHeaderData( c_tr, Qt::Horizontal, QObject::tr("t<sub>R</sub>(s)") );
    model.setHeaderData( c_area, Qt::Horizontal, QObject::tr("Area") );
    model.setHeaderData( c_height, Qt::Horizontal, QObject::tr("Height") );
    model.setHeaderData( c_width, Qt::Horizontal, QObject::tr("Width") );
    model.setHeaderData( c_ntp, Qt::Horizontal, QObject::tr("NTP") );
    model.setHeaderData( c_rs, Qt::Horizontal, QObject::tr("<i>Rs</i>") );
    model.setHeaderData( c_asymmetry, Qt::Horizontal, QObject::tr("Asymmetry") );
    model.setHeaderData( c_capacityfactor, Qt::Horizontal, QObject::tr("<i>k'<i>") );

    setColumnHidden( c_id, true );
    // resizeColumnsToContents();
}

void
PeakTable::OnFinalClose()
{
}

bool
PeakTable::getContents( boost::any& ) const
{
    return false;
}

bool
PeakTable::setContents( boost::any&& )
{
    return false;
}

void
PeakTable::setData( const adcontrols::Peaks& peaks )
{
    QStandardItemModel& model = *model_;
    model.removeRows( 0, model.rowCount() );

    using namespace adcontrols;
    for ( Peaks::vector_type::const_iterator it = peaks.begin(); it != peaks.end(); ++it )
        add( *it );
    // resizeColumnsToContents();
}

void
PeakTable::setData( const adcontrols::PeakResult& result )
{
    setData( result.peaks() );
}

void
PeakTable::add( const adcontrols::Peak& peak )
{
    QStandardItemModel& model = *model_;

    int row = model.rowCount();
    model.setRowCount( row + 1 );
    
    model.setData( model.index( row, c_id ), static_cast< int >( peak.peakId() ) );
    model.setData( model.index( row, c_name ), QString::fromStdString( peak.name() ) );
    model.setData( model.index( row, c_tr ), static_cast<double>( peak.peakTime() ), Qt::EditRole );
   // model.setData( model.index( row, c_tr ), static_cast<double>( adcontrols::timeutil::toMinutes( peak.peakTime() ) ), Qt::EditRole );
    model.setData( model.index( row, c_area ), peak.peakArea(), Qt::EditRole );
    model.setData( model.index( row, c_height ), peak.peakHeight(), Qt::EditRole );
    model.setData( model.index( row, c_width ), peak.peakWidth(), Qt::EditRole );
    model.setData( model.index( row, c_ntp ), peak.theoreticalPlate().ntp(), Qt::EditRole );
    model.setData( model.index( row, c_rs ), peak.resolution().resolution(), Qt::EditRole );
    model.setData( model.index( row, c_asymmetry ), peak.asymmetry().asymmetry(), Qt::EditRole );
    model.setData( model.index( row, c_capacityfactor ), peak.capacityFactor(), Qt::EditRole );

    for ( int column = 0; column < model.columnCount(); ++column ) {
        model.itemFromIndex( model.index( row, column ) )->setSelectable( true );
        model.itemFromIndex( model.index( row, column ) )->setEditable( true );
    }
    model.itemFromIndex( model.index( row, c_name ) )->setEditable( false );
}

void
PeakTable::currentChanged( const QModelIndex& curr, const QModelIndex& prev )
{
    if ( curr.row() != prev.row() ) {
        int peakId = model_->data( model_->index( curr.row(), c_id ) ).toInt();
        emit currentChanged( peakId );
    }
}

// #include "mschromatogramtable.moc"
