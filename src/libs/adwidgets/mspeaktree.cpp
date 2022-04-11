/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mspeaktree.hpp"
#include "htmlheaderview.hpp"
#include "delegatehelper.hpp"
#include "grid_render.hpp"
#include <adcontrols/annotations.hpp>
#include <adcontrols/annotation.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/genchromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/targeting.hpp>

#include <adportable/float.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/is_type.hpp>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QItemDelegate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QList>
#include <QStandardItemModel>
#include <QMenu>
#include <QPainter>
#include <QPair>
#include <QtPrintSupport/QPrinter>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/signals2.hpp>
#include <boost/variant.hpp>
#include <algorithm>
#include <ratio>
#include <set>
#include <sstream>

namespace adwidgets {

	using namespace adcontrols::metric;

    namespace {

        enum {
            c_display_name
            , c_formula
            , c_exact_mass
            , c_mass
            , c_mass_error
            , c_delta_mass
            , c_intensity
            , c_relative_intensity
            , c_exact_abundance
            , c_abundance_error
            , c_time
            , c_mode
            , c_index
            , c_fcn
            , c_num_columns
        };

        static QColor colors[] = {
            { QColor( 0xff, 0x66, 0x44, 0x20 ) }   // 1
            , { QColor( 0x6d, 0x79, 0x93, 0x20 ) } // 2 Lavendar
            , { QColor( 0x99, 0x09, 0xa2, 0x20 ) } // 3 Overcast
            , { QColor( 0xd5, 0xd5, 0xd5, 0x20 ) } // 4
            , { QColor( 0xca, 0xeb, 0xf2, 0x40 ) }
            , { QColor( 0xa9, 0xa9, 0xa9, 0x40 ) }
            , { QColor( 0xff, 0x38, 0x3f, 0x40 ) } // watermelon
            , { QColor( 0xef, 0xef, 0xef, 0x40 ) }
            , { QColor( 0xff, 0x66, 0x44, 0x90 ) }
            , { QColor( 0xff, 0x66, 0x44, 0xa0 ) }
            , { QColor( 0xff, 0x66, 0x44, 0xb0 ) }
        };

        class delegate : public QItemDelegate {
            Q_OBJECT
        public:
            explicit delegate( QObject *parent = 0 ) : QItemDelegate( parent ) {
            }

            void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
                QStyleOptionViewItem op( option );
                op.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

                int fcn = index.model()->data( index.model()->index( index.row(), c_fcn ) ).toInt();
                if ( fcn > 0 ) {
                    size_t cid = ( fcn - 1 ) % (sizeof( colors )/sizeof( colors[ 0 ] ));
                    painter->save();
                    painter->fillRect( option.rect, colors[ cid ] );
                    painter->restore();
                }

                bool valid = index.model()->data( index.model()->index( index.row(), c_index, index.parent() ), Qt::EditRole ).toInt() >= 0;
                if ( !valid ) {
                    painter->save();
                    painter->fillRect( option.rect, QColor{ 0x6d, 0x6d, 0x6d, 0x40 } );
                    painter->restore();
                }

                switch( index.column() ) {
                case c_display_name:
                    DelegateHelper::render_html( painter, option, index.data().toString() );
                    break;
                case c_time:
                    if ( valid ) {
                        op.displayAlignment = Qt::AlignRight | Qt::AlignHCenter;
                        drawDisplay( painter, op, option.rect
                                     , (boost::format( "%.5lf" ) % scale_to_micro( index.data( Qt::EditRole ).toDouble() )).str().c_str() );
                    }
                    break;
                case c_exact_mass:
                    drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'g', 7 ) );
                    break;
                case c_mass:
                    if ( valid )
                        drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'g', 7 ) );
                    else
                        drawDisplay( painter, op, option.rect, "-------" );
                    break;
                case c_mass_error:
                    if ( valid )
                        drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'g', 5 ) );
                    break;
                case c_intensity:
                    if ( valid )
                        drawDisplay( painter, op, option.rect, (boost::format( "%.2lf" ) % (index.data( Qt::EditRole ).toDouble())).str().c_str() );
                    break;
                case c_relative_intensity:
                case c_exact_abundance:
                    if ( valid )
                        drawDisplay( painter, op, option.rect, (boost::format( "%.4lf" ) % (index.data( Qt::EditRole ).toDouble())).str().c_str() );
                    break;
                case c_abundance_error:
                    if ( valid )
                        drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'g', 5 ) );
                    break;
                case c_formula:
                    do {
                        std::string formula = adcontrols::ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                        DelegateHelper::render_html( painter, option, QString::fromStdString( formula ) );
                    } while(0);
                    break;
                case c_mode:
                case c_num_columns:
                default:
                    QItemDelegate::paint( painter, option, index );
                    break;
                }
            }

            void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
                QItemDelegate::setModelData( editor, model, index );
                emit valueChanged( index );
            }

            QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                auto size = QItemDelegate::sizeHint(option, index);
                if ( index.column() == c_formula || index.column() == c_display_name ) {
                    size = DelegateHelper::html_size_hint( option, index );
                }
                if ( index.parent() == QModelIndex() )
                    size.setHeight(20);
                return size;
            }

        signals:
            void valueChanged( const QModelIndex& ) const;

        public slots:

        };


        template<class T> struct lock_weak_pointer : public boost::static_visitor< std::shared_ptr<T> > {
            std::shared_ptr< T > operator ()( std::weak_ptr<T>& wptr ) const {
                return wptr.lock();
            }
        };

        struct annotation_updator {

            bool operator () ( adcontrols::MassSpectrumPtr& ptr, int idx, int fcn, const std::string& formula ) {
                adcontrols::segment_wrapper<> segs( *ptr );
                if ( signed(segs.size()) > fcn ) {
                    auto& ms = segs[fcn];
                    if ( !formula.empty() ) {
                        ms.get_annotations() << adcontrols::annotation( formula
                                                                        , ms.mass( idx )
                                                                        , ms.intensity( idx )
                                                                        , idx
                                                                        , 0
                                                                        , adcontrols::annotation::dataFormula );
                    } else {
                        ms.get_annotations().erase_if( [&](const auto& a ){ return a.index() == idx && a.dataFormat() == adcontrols::annotation::dataFormula; });
                    }
                    return true;
                }
                return false;
            }

        };

        struct findRow {
            int operator()( QStandardItemModel& model, int idx, int fcn ) const {
                for ( int i = 0; i < model.rowCount(); ++i ) {
                    if ( model.index( i, c_index ).data( Qt::EditRole ).toInt() == idx &&
                         model.index( i, c_fcn ).data( Qt::EditRole ).toInt() == fcn )
                        return i;
                }
                return (-1);
            }
        };

        // compare QModelIndex for tree structure
        struct compare {
            bool operator()( const QModelIndex& a, const QModelIndex& b ) const {
                auto r1 = a.parent() == QModelIndex() ? std::make_pair( a.row(), -1 ) : std::make_pair( a.parent().row(), a.row() );
                auto r2 = b.parent() == QModelIndex() ? std::make_pair( b.row(), -1 ) : std::make_pair( b.parent().row(), b.row() );
                if ( r1 == r2 )
                    return a.column() < b.column();
                return r1 < r2;
            }
        };

    }
}

namespace adwidgets {

    class MSPeakTree::impl {
    public:
        std::unique_ptr< QStandardItemModel > model_;
        std::unique_ptr< QItemDelegate > delegate_;

        impl() : model_( std::make_unique< QStandardItemModel >() )
               , delegate_( std::make_unique< delegate >() ) {
        }

        std::weak_ptr< adcontrols::MassSpectrum > data_source_;
        std::weak_ptr< adcontrols::MSPeakInfo > pkinfo_;  // it is a pair of data_source_
        std::weak_ptr< const adcontrols::Targeting > targeting_;

        boost::signals2::signal< callback_t > callback_;
        bool inProgress_;
    };
}

using namespace adwidgets;

MSPeakTree::~MSPeakTree()
{
    delete impl_;
}

MSPeakTree::MSPeakTree(QWidget *parent) : QTreeView( parent )
                                        , impl_( new impl() )
{
    //this->setHorizontalHeader( new HtmlHeaderView );
    this->setHeader( new HtmlHeaderView );
    this->setModel( impl_->model_.get() );
	this->setItemDelegate( impl_->delegate_.get() );
    this->setSortingEnabled( true );
    //this->verticalHeader()->setDefaultSectionSize( 18 );
    this->setContextMenuPolicy( Qt::CustomContextMenu );

    this->setSelectionMode( QAbstractItemView::ExtendedSelection );

    connect( this, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( showContextMenu( const QPoint& ) ) );
    connect( impl_->delegate_.get(), SIGNAL( valueChanged( const QModelIndex& ) ), this, SLOT( handleValueChanged( const QModelIndex& ) ) );
}

void *
MSPeakTree::query_interface_workaround( const char * typenam )
{
    if ( typenam == typeid( MSPeakTree ).name() )
        return static_cast< MSPeakTree * >(this);
    return 0;
}

void
MSPeakTree::OnCreate( const adportable::Configuration& )
{
}

void
MSPeakTree::OnInitialUpdate()
{
    onInitialUpdate();
}

void
MSPeakTree::onUpdate( boost::any&& a )
{
    if ( adportable::a_type< adcontrols::MassSpectrumPtr >::is_a( a ) ) {
        // lockMassHandled on MainWindow invoke this method

        auto ptr = boost::any_cast< adcontrols::MassSpectrumPtr >( a );
        auto wptr = boost::get< std::weak_ptr< adcontrols::MassSpectrum > >( impl_->data_source_ );
        if ( wptr.lock() == ptr )
            updateData( *ptr );
        else
            setData( *ptr );

    } else if ( a.type() == typeid(int) ) {
        // dataMayChanged on MainWindow invoke this method over applyCalibration()

        //int id = boost::any_cast<int>( a );

        // if ( id == 0 ) { // data may changed
        //     boost::apply_visitor( detail::dataMayChanged( this ), impl_->data_source_ );
        // }
    }
}

void
MSPeakTree::dataMayChanged()
{
    ADDEBUG() << "dataMayChanged";
    //boost::apply_visitor( detail::dataMayChanged( this ), impl_->data_source_ );
}

void
MSPeakTree::OnFinalClose()
{
}

bool
MSPeakTree::getContents( boost::any& ) const
{
    return false;
}

bool
MSPeakTree::setContents( boost::any&& a )
{
    return false;
}

void
MSPeakTree::setContents( std::shared_ptr< adcontrols::MassSpectrum > ms, std::function< callback_t > callback )
{
}

///////////// hire is the main entry for set up target result
void
MSPeakTree::setContents( std::tuple< std::shared_ptr< adcontrols::MSPeakInfo >
                         , std::shared_ptr< adcontrols::MassSpectrum >
                         , std::shared_ptr< const adcontrols::Targeting > >&& tuple )
{
    impl_->pkinfo_ = std::move( std::get< 0 >( tuple ) );
    auto ms = std::move( std::get< 1 >( tuple ) );
    impl_->data_source_ = ms;
    auto target = std::move( std::get< 2 >( tuple ) );
    setPeakInfo( *target, ms );
}

void
MSPeakTree::setContents( std::pair< std::shared_ptr< adcontrols::MassSpectrum >, std::shared_ptr< const adcontrols::Targeting > >&& pair )
{
    impl_->pkinfo_.reset();
    auto ms = std::move( pair.first );
    auto target = std::move( pair.second );

    if ( ms && target ) {
        impl_->data_source_ = ms;
        setPeakInfo( *target, ms );
    } else {
        impl_->model_->setRowCount( 0 );
    }
}


QStandardItemModel&
MSPeakTree::model()
{
    return *impl_->model_;
}

void
MSPeakTree::onInitialUpdate()
{
    QStandardItemModel& model = *impl_->model_;

    model.setColumnCount( c_num_columns );
    model.setHeaderData( c_display_name, Qt::Horizontal, QObject::tr( "Name" ) );
    model.setHeaderData( c_time,        Qt::Horizontal, QObject::tr( "time(&mu;s)" ) );
    model.setHeaderData( c_exact_mass,  Qt::Horizontal, QObject::tr( "Exact <i>m/z</i>" ) );
    model.setHeaderData( c_mass,        Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
    model.setHeaderData( c_mass_error,  Qt::Horizontal, QObject::tr( "error(mDa)" ) );
    model.setHeaderData( c_delta_mass,  Qt::Horizontal, QObject::tr( "&delta;Da" ) );
    model.setHeaderData( c_intensity,   Qt::Horizontal, QObject::tr( "Abundance" ) );
    model.setHeaderData( c_relative_intensity, Qt::Horizontal, QObject::tr( "R. A. (%)" ) );
    model.setHeaderData( c_exact_abundance,    Qt::Horizontal, QObject::tr( "R. A. (exact,%)" ) );
    model.setHeaderData( c_abundance_error,    Qt::Horizontal, QObject::tr( "R. A. Error (%)" ) );
    model.setHeaderData( c_mode,        Qt::Horizontal, QObject::tr( "mode" ) );
    model.setHeaderData( c_formula,     Qt::Horizontal, QObject::tr( "formula" ) );

    setColumnHidden( c_index, true );
    setColumnHidden( c_fcn, true );  // a.k.a. protocol id, internally used as an id
}

void
MSPeakTree::setPeakInfo( const adcontrols::Targeting& targeting, std::shared_ptr< const adcontrols::MassSpectrum > ms )
{
    QStandardItemModel& model = *impl_->model_;
    const auto& candidates = targeting.candidates();

    model.setRowCount( 0 );
    if ( candidates.empty() || !ms )
        return;

    model.setRowCount( candidates.size() );

    size_t matchCount(0);

    size_t row(0);
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > v_ms(*ms);

    for ( auto& c: candidates ) {
        model.setData( model.index( row, c_display_name ), QString::fromStdString( c.display_name ) );

        model.setData( model.index( row, c_fcn ), c.fcn ); // hidden
        model.setData( model.index( row, c_index ), c.idx );

        model.setData( model.index( row, c_time ), v_ms[ c.fcn ].time( c.idx ) );
        model.setData( model.index( row, c_mass ), v_ms[ c.fcn ].mass( c.idx ) );
        model.setData( model.index( row, c_intensity ), v_ms[ c.fcn ].intensity( c.idx ) );

        model.setData( model.index( row, c_relative_intensity ), 100.0 ); // (abundance * 100) / iMax );
        model.setData( model.index( row, c_mode ), v_ms[ c.fcn ].mode() );

        //
        model.setData( model.index( row, c_formula ), QString::fromStdString( c.formula ) );
        model.setData( model.index( row, c_exact_mass ), c.exact_mass );
        model.setData( model.index( row, c_mass_error ), (c.mass - c.exact_mass) * 1000 );

        model.setData( model.index( row, c_exact_abundance ), 100.0 );
        model.setData( model.index( row, c_abundance_error ), "n/a" );
        setRowHidden( row, QModelIndex(), false );
        matchCount++;

        // -- sub tree
        auto parent = model.itemFromIndex( model.index( row, 0 ) );
        parent->setColumnCount( c_num_columns );
        parent->setRowCount( c.isotopes.size() );

        size_t iRow = 0;
        for ( auto i: c.isotopes ) {

            if ( auto item = model.itemFromIndex( model.index( iRow, c_formula, parent->index() ) ) ) {

                model.setData( model.index( iRow, c_fcn, parent->index() ), c.fcn ); // hidden
                model.setData( model.index( iRow, c_index, parent->index() ), i.idx );
                model.setData( model.index( iRow, c_exact_mass, parent->index() ), i.exact_mass );
                if ( i.idx >= 0 ) {
                    model.setData( model.index( iRow, c_mass, parent->index() ), i.mass );
                    model.setData( model.index( iRow, c_mass_error, parent->index() ), (i.mass - i.exact_mass) * 1000 );
                    model.setData( model.index( iRow, c_intensity, parent->index() ), v_ms[ c.fcn ].intensity( i.idx ) );
                    model.setData( model.index( iRow, c_relative_intensity, parent->index() )
                                   , 100 * ( v_ms[ c.fcn ].intensity( i.idx ) / v_ms[ c.fcn ].intensity( c.idx ) ) );
                    model.setData( model.index( iRow, c_abundance_error, parent->index() ), 100 * i.abundance_ratio_error );
                    model.setData( model.index( iRow, c_time, parent->index() ), v_ms[ c.fcn ].time( i.idx ) );
                } else {
                    model.setData( model.index( iRow, c_mass, parent->index() ), QVariant() );
                    model.setData( model.index( iRow, c_mass_error, parent->index() ), QVariant() );
                    model.setData( model.index( iRow, c_intensity, parent->index() ), QVariant() );
                    model.setData( model.index( iRow, c_relative_intensity, parent->index() ), QVariant() );
                    model.setData( model.index( iRow, c_abundance_error, parent->index() ), QVariant() );
                    model.setData( model.index( iRow, c_time, parent->index() ), QVariant() );
                }
                model.setData( model.index( iRow, c_exact_abundance, parent->index() ), 100 * i.exact_abundance );
            }
            ++iRow;
        }
        row++;
    }

    resizeColumnToContents( c_formula );
    resizeColumnToContents( c_display_name );
}


void
MSPeakTree::setData( const adcontrols::MassSpectrum& ms )
{
    // adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    // size_t total_size = 0;
    // for( auto& t: segs )
    //     total_size += t.size();

    // //setPeakInfo( ms );
    return;
}

void
MSPeakTree::updateData( const adcontrols::MassSpectrum& ms )
{
    // call from lockmass via onUpdate
    // nothing to be done
}

void
MSPeakTree::currentChanged( const QModelIndex& index, const QModelIndex& prev )
{
    QStandardItemModel& model = *impl_->model_;
    (void)prev;

    scrollTo( index, QAbstractItemView::EnsureVisible );

	int row = index.row();

    setUpdatesEnabled( false );

    double mass = model.index( index.row(), c_mass, index.parent() ).data( Qt::EditRole ).toDouble();

    for ( int r = 0; r < model.rowCount(); ++r ) {
        double d = std::abs( model.index( r, c_mass ).data( Qt::EditRole ).toDouble() - mass );
        model.setData( model.index( r, c_delta_mass ), int( d + 0.7 ) );

        if ( auto parent = model.itemFromIndex( model.index( r, 0 ) ) ) {
            for ( auto i = 0; i < parent->rowCount(); ++i ) {
                double d = std::abs( model.index( i, c_exact_mass, parent->index() ).data( Qt::EditRole ).toDouble() - mass );
                model.setData( model.index( i, c_delta_mass, parent->index() ), int( d + 0.7 ) );
            }
        }
    }

    setUpdatesEnabled( true );

    int idx = model.index( row, c_index ).data( Qt::EditRole ).toInt();
    int fcn = model.index( row, c_fcn ).data( Qt::EditRole ).toInt();
    emit currentChanged( idx, fcn );
}


void
MSPeakTree::keyPressEvent( QKeyEvent * event )
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
MSPeakTree::handleZoomedOnSpectrum( const QRectF& rc, int axis )
{
    QStandardItemModel& model = *impl_->model_;

    bool isTimeAxis = axis == adcontrols::hor_axis_time;

    if ( auto ptr = impl_->data_source_.lock() ) {
        std::pair<int, int> bp = adcontrols::segments_helper::base_peak_index( *ptr, rc.left(), rc.right(), isTimeAxis ); // index,fcn

        if ( bp.first >= 0 && bp.second >= 0 ) {
            do {
                // ---> change rel. intensity
                setUpdatesEnabled( false );
                double base_height = adcontrols::segments_helper::get_intensity( *ptr, bp );
                for ( int row = 0; row < model.rowCount(); ++row ) {
                    model.setData( model.index( row, c_relative_intensity )
                                   , model.index( row, c_intensity ).data().toDouble() * 100 / base_height );
                }
                setUpdatesEnabled( true );
                // <--- end rel. intensity
            } while ( 0 );

            do {
                for ( int row = 0; row < model.rowCount(); ++row ) {
                    if ( model.index( row, c_index ).data( Qt::EditRole ).toInt() == bp.first
                         && model.index( row, c_fcn ).data( Qt::EditRole ).toInt() == bp.second ) {

                        QModelIndex index = model.index( row, isTimeAxis ? c_time : c_mass );
                        setCurrentIndex( index );
                        scrollTo( index, QAbstractItemView::EnsureVisible );
                        break;
                    }
                }
            } while ( 0 );
        }
    }
}

void
MSPeakTree::handleCopyAllToClipboard()
{
    QStandardItemModel& model = *impl_->model_;

    QString selected_text;

    for ( int row = 0; row < model.rowCount(); ++row ) {
        auto formula = model.data( model.index( row, c_formula ) ).toString();
        selected_text.append( QString( "\"%1\"\t" ).arg( formula ) );
        for ( int col = 1; col < model.columnCount(); ++col ) {
            selected_text.append( model.index( row, col ).data( Qt::EditRole ).toString() );
            if ( col != model.columnCount() - 1 )
                selected_text.append( '\t' );
        }
        selected_text.append( '\n' );

        if ( auto parent = model.itemFromIndex( model.index( row, 0 ) ) ) {
            for ( auto i = 0; i < parent->rowCount(); ++i ) {
                selected_text.append( '\t' ); // empty for formula
                for ( int col = 1; col < model.columnCount(); ++col ) {
                    selected_text.append( model.index( i, col, parent->index() ).data( Qt::EditRole ).toString() );
                    if ( col != model.columnCount() - 1 )
                        selected_text.append( '\t' );
                }
                selected_text.append( '\n' );
            }
        }
    }
    QApplication::clipboard()->setText( selected_text );
}


void
MSPeakTree::handleCopyToClipboard()
{
    QStandardItemModel& model = *impl_->model_;
    QModelIndexList list = selectionModel()->selectedRows();

    if ( list.size() < 1 )
        return;

    std::sort( list.begin(), list.end(), []( const auto& a, const auto& b ){ return compare()( a, b ); } );

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
MSPeakTree::showContextMenu( const QPoint& pt )
{
    QStandardItemModel& model = *impl_->model_;
    QModelIndex index = currentIndex();

	if ( index.isValid() ) {

        QMenu menu;

        QModelIndexList list = selectionModel()->selectedIndexes();
        if ( list.size() < 1 )
            return;

        //--------------------
        menu.addAction( tr("Gen. Chromatogram(s) ..."), [=,this]{ handleGenChromatogram(); } );

        std::set< int > rows;
        for ( auto index: list )
            rows.insert( index.row() ); // make unique row list

        //----------- gather references ----------
        QString formulae;
        QVector< QPair<int, int> > refs;

        for ( int row: rows ) {

            QString formula = model.data( model.index( row, c_formula ) ).toString();

            if ( ! formula.isEmpty() ) {
                formulae += QString( (refs.isEmpty() ? "%1" : ", %1") ).arg( formula );
                int idx = model.data( model.index( row, c_index ) ).toInt();
                int fcn = model.data( model.index( row, c_fcn ) ).toInt();

                refs.push_back( QPair<int,int>( idx, fcn ) );
            }
        }

        //------------ lock mass
        if ( impl_->callback_.empty() )
            menu.addAction( tr("Lock mass with %1").arg( formulae ), [=,this](){ emit triggerLockMass( refs ); } );
        else
            menu.addAction( tr("Lock mass with %1").arg( formulae ), [=,this](){ impl_->callback_( lockmass_triggered, refs ); } ); // for SpectrogramWnd

        //------------ Copy assigned
        menu.addAction( tr("Copy All"), this, SLOT( handleCopyAllToClipboard() ) );
        menu.addAction( tr("Copy Selected"), this, SLOT( handleCopyToClipboard() ) );

        //-- add dataprocessor dependent menu --
        if ( auto ptr = impl_->data_source_.lock() ) {
            addContextMenu( menu, pt, ptr );
            menu.addSeparator();
        }

        menu.exec( this->mapToGlobal( pt ) );
    }
}

void
MSPeakTree::handleValueChanged( const QModelIndex& index )
{
    if ( impl_->inProgress_ )
        return;
    if ( index.column() == c_formula ) {
        formulaChanged( index );
    }
}

void
MSPeakTree::formulaChanged( const QModelIndex& index )
{
}

void
MSPeakTree::descriptionChanged( const QModelIndex& index )
{
}

double
MSPeakTree::exactMass( std::string formula )
{
    if ( formula.empty() )
        return 0;
    auto neutral = adcontrols::ChemicalFormula::neutralize( formula );
    return adcontrols::ChemicalFormula().getMonoIsotopicMass( adcontrols::ChemicalFormula::split( neutral.first ) ).first;
}

bool
MSPeakTree::getMSPeak( adcontrols::MSPeak& peak, int row ) const
{
	QStandardItemModel& model = *impl_->model_;

    peak.time( model.index( row, c_time ).data( Qt::EditRole ).toDouble() );
    peak.mass( model.index( row, c_mass ).data( Qt::EditRole ).toDouble() );
    peak.mode( model.index( row, c_mode ).data( Qt::EditRole ).toInt() );

    peak.fcn( model.index( row, c_fcn ).data( Qt::EditRole ).toInt() );
    //peak.width( double, bool isTime = false );
    //peak.exit_delay( double );
    //peak.flight_length( double );
    peak.formula( model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdString() );
    peak.description( model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdWString() );
    peak.spectrumIndex( model.index( row, c_index ).data( Qt::EditRole ).toInt() );

    return true;
}

bool
MSPeakTree::getMSPeaks( adcontrols::MSPeaks& peaks, GETPEAKOPTS opt ) const
{
    if ( opt == SelectedPeaks ) {

        QModelIndexList list = selectionModel()->selectedIndexes();
        if ( list.size() < 1 )
            return false;

        std::set< int > rows;
        for ( auto index: list )
            rows.insert( index.row() ); // make unique row list

        for ( auto& row: rows ) {
            adcontrols::MSPeak pk;
            if ( getMSPeak( pk, row ) )
                peaks << pk;
        }

    } else if ( opt == AssignedPeaks ) {

        for (int row = 0; row < impl_->model_->rowCount(); ++row ) {
            adcontrols::MSPeak pk;
            if ( getMSPeak( pk, row ) ) {
                if ( !pk.formula().empty() )
                    peaks << pk;
            }
        }

    } else {

        for (int row = 0; row < impl_->model_->rowCount(); ++row ) {
            adcontrols::MSPeak pk;
            if ( getMSPeak( pk, row ) )
                peaks << pk;
        }
    }
    return true;
}

int
MSPeakTree::findColumn( const QString& name ) const
{
    int nColumn = impl_->model_->columnCount();
    for ( int col = 0; col < nColumn; ++col ) {
        if ( impl_->model_->headerData( col, Qt::Horizontal, Qt::EditRole ).toString() == name )
            return col;
    }
    return -1;
}

void
MSPeakTree::handlePrint( QPrinter& printer, QPainter& painter )
{
    const QStandardItemModel& model = *(impl_->model_);
    printer.newPage();
    auto pageRect = printer.pageLayout().paintRectPixels( printer.resolution() );
	const QRect rect( pageRect.x() + pageRect.width() * 0.05
                      , pageRect.y() + pageRect.height() * 0.05
                      , pageRect.width() * 0.9, pageRect.height() * 0.8 );

    const int rows = model.rowCount();
    const int cols = model.columnCount();

    grid_render render( rect );

    for ( int col = 0; col < cols; ++col ) {
        double width = 0;
        switch( col ) {
        case c_formula:              width = rect.width() / 180 * 40; break;
        case c_exact_mass:           width = rect.width() / 180 * 30; break;
        case c_mass:                 width = rect.width() / 180 * 30; break;
        case c_mass_error:           width = rect.width() / 180 * 24; break;
        case c_intensity:            width = rect.width() / 180 * 24; break;
        case c_relative_intensity:   width = rect.width() / 180 * 24; break;
        case c_time:                 width = rect.width() / 180 * 18; break;
        }
        render.add_tab( width );
    }

    render.new_page( painter );
    for ( int col = 0; col < cols; ++col ) {
        render( painter, col, model.headerData( col, Qt::Horizontal ).toString(), "center" );
    }

    render.new_line( painter );
    render.draw_horizontal_line( painter );

    for ( int row = 0; row < rows; ++row ) {

        if ( isRowHidden( row, QModelIndex() ) )
            continue;

        QString formula = model.index( row, c_formula ).data( Qt::EditRole ).toString();

        for ( int col = 0; col < cols; ++col ) {
            auto data = model.index( row, col ).data( Qt::EditRole );
            QString text;
            switch( col ) {
            case c_formula:
                text = QString::fromStdString( adcontrols::ChemicalFormula::formatFormulae( data.toString().toStdString() ) );
                render( painter, col, text, "left" );
                break;
            case c_exact_mass:
                text = formula.isEmpty() ? QString() : QString::number( data.toDouble(), 'g', 7 );
                render( painter, col, text );
                break;
            case c_mass:
                text = QString::number( data.toDouble(), 'g', 7 );
                render( painter, col, text );
                break;
            case c_mass_error:
                text = QString::number( ( data.toDouble() * std::milli::den ), 'g', 7 );
                render( painter, col, text );
                break;
            case c_intensity:
                text = QString::number( data.toDouble(), 'g', 1 );
                render( painter, col, text );
                break;
            case c_relative_intensity:
                text = QString::number( data.toDouble(), 'g', 4 );
                render( painter, col, text );
                break;
            case c_time:
                text = QString::number( data.toDouble(), 'g', 4 );
                render( painter, col, text );
                break;
            }
        }

        if ( render.new_line( painter ) ) {
            printer.newPage();
            render.new_page( painter );
            for ( int col = 0; col < cols; ++col )
                render( painter, col, model.headerData( col, Qt::Horizontal ).toString(), "center" );
            render.new_line( painter );
            render.draw_horizontal_line( painter );
        }
    }
    render.draw_horizontal_line( painter );

}

void
MSPeakTree::handleGenChromatogram() const
{
    QModelIndexList list = selectionModel()->selectedRows();
    if ( list.size() < 1 )
        return;

    std::sort( list.begin(), list.end(), []( const auto& a, const auto& b ){ return compare()( a, b ); } );

    const auto& model = *impl_->model_;
    QJsonArray a;

    std::vector< adcontrols::GenChromatogram > genChromatograms;

    auto it = list.begin();
    while ( it != list.end() ) {
        // QJsonObject obj;
        QModelIndex topIndex = ( it->parent() == QModelIndex() ) ? (*it) : it->parent();

        // obj = QJsonObject{
        //     { "formula", model.index( topIndex.row(), c_formula ).data( Qt::EditRole ).toString() }
        //     , { "display_name", model.index( topIndex.row(), c_display_name ).data( Qt::EditRole ).toString() }
        //     , { "exact_mass", model.index( topIndex.row(), c_exact_mass ).data( Qt::EditRole ).toDouble() }
        //     , { "exact_abundance", model.index( topIndex.row(), c_exact_abundance ).data( Qt::EditRole ).toDouble() }
        //     , { "mass", model.index( topIndex.row(), c_mass ).data( Qt::EditRole ).toDouble() }
        //     , { "time", model.index( topIndex.row(), c_time ).data( Qt::EditRole ).toDouble() }
        //     , { "index", model.index( topIndex.row(), c_index ).data( Qt::EditRole ).toInt() }
        //     , { "proto", model.index( topIndex.row(), c_fcn ).data( Qt::EditRole ).toInt() }
        //     , { "selected", ( it->parent() == QModelIndex() ? true : false ) }
        // };

        // formulae
        adcontrols::GenChromatogram g;
        g.formula         = model.index( topIndex.row(), c_formula ).data( Qt::EditRole ).toString().toStdString();
        g.display_name    = model.index( topIndex.row(), c_display_name ).data( Qt::EditRole ).toString().toStdString();
        g.exact_mass      = model.index( topIndex.row(), c_exact_mass ).data( Qt::EditRole ).toDouble();
        g.exact_abundance = model.index( topIndex.row(), c_exact_abundance ).data( Qt::EditRole ).toDouble();
        g.mass            = model.index( topIndex.row(), c_mass ).data( Qt::EditRole ).toDouble();
        g.time            = model.index( topIndex.row(), c_time ).data( Qt::EditRole ).toDouble();
        g.index           = model.index( topIndex.row(), c_index ).data( Qt::EditRole ).toInt();
        g.proto           = model.index( topIndex.row(), c_fcn ).data( Qt::EditRole ).toInt();
        g.selected        = it->parent() == QModelIndex() ? true : false;

        if ( it->parent() == QModelIndex() )
            std::advance( it, 1 );

        QJsonArray sub;
        while ( it != list.end() && ( it->parent() == topIndex ) ) {
            // QJsonObject sobj {
            //     { "exact_mass",        model.index( it->row(), c_exact_mass, it->parent() ).data( Qt::EditRole ).toDouble() }
            //     , { "exact_abundance", model.index( it->row(), c_exact_abundance, it->parent() ).data( Qt::EditRole ).toDouble() }
            //     , { "mass",            model.index( it->row(), c_mass, it->parent() ).data( Qt::EditRole ).toDouble() }
            //     , { "time",            model.index( it->row(), c_time, it->parent() ).data( Qt::EditRole ).toDouble() }
            //     , { "index",           model.index( it->row(), c_index, it->parent() ).data( Qt::EditRole ).toInt() }
            //     , { "proto",           model.index( it->row(), c_fcn, it->parent() ).data( Qt::EditRole ).toInt() }
            //     , { "selected", true }
            // };
            // sub.push_back( sobj );
            adcontrols::targeting::isotope iso;
            iso.idx             = model.index( it->row(), c_index, it->parent() ).data( Qt::EditRole ).toInt();
            iso.mass            = model.index( it->row(), c_mass, it->parent() ).data( Qt::EditRole ).toDouble();
            iso.abundance_ratio = model.index( it->row(), c_exact_abundance, it->parent() ).data( Qt::EditRole ).toDouble();
            iso.abundance_ratio_error = model.index( it->row(), c_abundance_error, it->parent() ).data( Qt::EditRole ).toDouble() / 100.;
            iso.exact_mass      = model.index( it->row(), c_exact_mass, it->parent() ).data( Qt::EditRole ).toDouble();
            iso.exact_abundance = model.index( it->row(), c_exact_abundance, it->parent() ).data( Qt::EditRole ).toDouble();
            g.isotopes.emplace_back( std::move( iso ) );

            std::advance( it, 1 );
        }

        // obj[ "children" ] = sub;
        // a.push_back( obj );

        genChromatograms.emplace_back( g );
    }

    QJsonObject top{ { "formulae", a } };
    auto jv = boost::json::value_from( boost::json::object{ {"formulae", genChromatograms }} );
    auto json = boost::json::serialize( jv );
//#if !defined NDEBUG && 0
    // ADDEBUG() << QJsonDocument( top ).toJson( QJsonDocument::Indented ).toStdString();
//#endif
    emit generateChromatogram( QByteArray( json.data(), json.size() ) );
    // emit generateChromatogram( QJsonDocument( top ).toJson() );
}

void
MSPeakTree::addContextMenu(QMenu &, const QPoint &, std::shared_ptr<const adcontrols::MassSpectrum>) const
{
}


#include "mspeaktree.moc"
