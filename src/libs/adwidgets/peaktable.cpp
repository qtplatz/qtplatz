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
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <functional>
#include <set>

namespace {
    static QColor colors [] = {
        QColor( 0x00, 0x00, 0xff, 0x20 )    // 0  blue
        , QColor( 0xff, 0x00, 0x00, 0x20 )  // 1  red
        , QColor( 0x00, 0x80, 0x00, 0x20 )  // 2  green
        , QColor( 0x4b, 0x00, 0x82, 0x20 )  // 3  indigo
        , QColor( 0xff, 0x14, 0x93, 0x20 )  // 4  deep pink
        , QColor( 0x94, 0x00, 0xd3, 0x20 )  // 5  dark violet
        , QColor( 0x80, 0x00, 0x80, 0x20 )  // 6  purple
        , QColor( 0xdc, 0x13, 0x4c, 0x20 )  // 7  crimson
        , QColor( 0x69, 0x69, 0x69, 0x20 )  // 8  dim gray
        , QColor( 0x80, 0x80, 0x80, 0x20 )  // 9  gray
        , QColor( 0xa9, 0xa9, 0xa9, 0x20 )  //10  dark gray
        , QColor( 0xc0, 0xc0, 0xc0, 0x20 )  //11  silver
        , QColor( 0xd3, 0xd3, 0xd3, 0x20 )  //12  light gray
        , QColor( 0xd2, 0x69, 0x1e, 0x20 )  //13  chocolate
        , QColor( 0x00, 0x00, 0x8b, 0x20 )  //14  dark blue
        , QColor( 0xff, 0xff, 0xff, 0x20 )  //15  white
        , QColor( 0xff, 0x8c, 0x00, 0x20 )  //16  dark orange
        , QColor( 0x00, 0x00, 0x00, 0x00 )  //17
    };
}

namespace adwidgets {
    namespace peaktable {

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

                auto color = index.model()->index( index.row(), c_cid ).data( Qt::EditRole ).toInt();
                if ( color > 0 ) {
                    painter->save();
                    painter->fillRect( option.rect, colors[ color % (sizeof(colors)/sizeof(colors[0])) ] );
                    painter->restore();
                }
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
    setModel( model_ );
    setHorizontalHeader( new HtmlHeaderView( Qt::Horizontal, this ) );

    if ( auto delegate = new peaktable::ItemDelegate() ) {
        delegate->valueChanged_ = [this] ( const QModelIndex& idx ){
            emit valueChanged( model_->index( idx.row(), c_id ).data().toInt(), model_->index( idx.row(), c_cid).data().toInt(), idx );
        };
        setItemDelegate( delegate );
    }

    setSortingEnabled( true );

    connect( this, &TableView::rowsAboutToBeRemoved, [&]( const std::set< int >& rows ){
        std::vector< std::pair< int, int > > ids;
        for ( auto row: rows ) {
            ids.emplace_back( model_->index( row, c_cid ).data().toInt(), model_->index( row, c_id ).data().toInt() );
        }
        emit peaksAboutToBeRemoved( ids );
    });
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

    model.setHeaderData( c_id,             Qt::Horizontal, QObject::tr("Id") );
    model.setHeaderData( c_name,           Qt::Horizontal, QObject::tr("Name") );
    model.setHeaderData( c_tr,             Qt::Horizontal, QObject::tr("t<sub>R</sub>(s)") );
    model.setHeaderData( c_area,           Qt::Horizontal, QObject::tr("Area") );
    model.setHeaderData( c_height,         Qt::Horizontal, QObject::tr("Height") );
    model.setHeaderData( c_width,          Qt::Horizontal, QObject::tr("Width") );
    model.setHeaderData( c_ntp,            Qt::Horizontal, QObject::tr("NTP") );
    model.setHeaderData( c_rs,             Qt::Horizontal, QObject::tr("<i>Rs</i>") );
    model.setHeaderData( c_asymmetry,      Qt::Horizontal, QObject::tr("Asymmetry") );
    model.setHeaderData( c_capacityfactor, Qt::Horizontal, QObject::tr("<i>k'<i>") );
    model.setHeaderData( c_cid,            Qt::Horizontal, QObject::tr("overlay#") );  // chromatogram id

    setColumnHidden( c_id, true );
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
PeakTable::addData( adcontrols::PeakResult&& result, size_t idx, bool clearAll )
{
    if ( clearAll ) {
        model_->removeRows( 0, model_->rowCount() );
    } else {
        std::set< int > rows;
        for ( int row = 0; row < model_->rowCount(); ++row ) {
            if ( model_->index( row, c_cid ).data().toInt() == idx )
                rows.insert( row );
        }
        for ( auto it = rows.rbegin(); it != rows.rend(); ++it ) // remove bottom to top
            model_->removeRow( *it );
    }

    for ( const auto& peak: result.peaks() ) {
        add( peak, idx );
    }
    resizeColumnsToContents();
}

void
PeakTable::setData( const adcontrols::Peaks& peaks, bool isCounting )
{
    QStandardItemModel& model = *model_;
    model.removeRows( 0, model.rowCount() );

    model.setHeaderData( c_area, Qt::Horizontal, isCounting ? QObject::tr("Area(Counts)") : QObject::tr("Area") );

    for ( const auto& peak: peaks ) {
        add( peak, 0 );
    }
    resizeColumnsToContents();
    if ( peaks.size() >= 1 ) {
        auto it = std::max_element( peaks.begin(), peaks.end()
                                    , [](const auto& a, const auto& b){ return a.peakHeight() < b.peakHeight(); } );
        this->selectRow( std::distance( peaks.begin(), it ) );
    }
}

void
PeakTable::setData( const adcontrols::PeakResult& result )
{
    setData( result.peaks(), result.isCounting() );
}

void
PeakTable::add( const adcontrols::Peak& peak, size_t idx )
{
    QStandardItemModel& model = *model_;

    int row = model.rowCount();
    model.setRowCount( row + 1 );

    model.setData( model.index( row, c_cid ),            int(idx),             Qt::EditRole );
    model.setData( model.index( row, c_id ),             int( peak.peakId() ), Qt::EditRole );
    model.setData( model.index( row, c_name ),           QString::fromStdString( peak.name() ), Qt::EditRole );
    model.setData( model.index( row, c_tr ),             static_cast<double>( peak.peakTime() ), Qt::EditRole );
    model.setData( model.index( row, c_area ),           peak.peakArea(),      Qt::EditRole );
    model.setData( model.index( row, c_height ),         peak.peakHeight(),    Qt::EditRole );
    model.setData( model.index( row, c_width ),          peak.peakWidth(),     Qt::EditRole );
    model.setData( model.index( row, c_ntp ),            peak.theoreticalPlate().ntp(), Qt::EditRole );
    model.setData( model.index( row, c_rs ),             peak.resolution().resolution(), Qt::EditRole );
    model.setData( model.index( row, c_asymmetry ),      peak.asymmetry().asymmetry(), Qt::EditRole );
    model.setData( model.index( row, c_capacityfactor ), peak.capacityFactor(), Qt::EditRole );

    for ( auto col: { c_cid, c_id, c_tr, c_area, c_height, c_width, c_ntp, c_rs, c_asymmetry, c_capacityfactor } ) {
        model.itemFromIndex( model.index( row, col ) )->setEditable( false );
    }

    for ( auto col: { c_name } ) {
        model.itemFromIndex( model.index( row, col ) )->setEditable( idx == 0 ); // only current focused cid can be editted
    }

}

void
PeakTable::currentChanged( const QModelIndex& curr, const QModelIndex& prev )
{
    if ( curr.row() != prev.row() ) {
        int peakId = model_->data( model_->index( curr.row(), c_id ) ).toInt();
        emit currentChanged( peakId );
    }
}

// void
// PeakTable::rowsAboutToBeRemoved( const std::set< int >& rows )
// {
//     QVector< int > xrows;

//     for ( const auto row: rows ) {
//         auto cid = model_->index( row, c_cid ).data().toInt();
//         auto id  = model_->index( row, c_id ).data().toInt();
//         if ( cid == 0 ) {
//             ADDEBUG() << "row: " << row << ", " << std::make_pair( cid, id ) << " to be removed";
//             // xrows.push_back( QPair<int,int>(model_->index( row, c_cid ).data().toInt(), model_->index( row, c_id ).data().toInt()) );
//         }
//     }
// }

// #include "mschromatogramtable.moc"
