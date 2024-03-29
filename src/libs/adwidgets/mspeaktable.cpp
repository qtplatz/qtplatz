/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mspeaktable.hpp"
#include "htmlheaderview.hpp"
#include "delegatehelper.hpp"
#include "grid_render.hpp"
#include <QtCore/qobject.h>
#include <adcontrols/annotations.hpp>
#include <adcontrols/annotation.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lapfinder.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/targeting.hpp>
#include <adutils/constants.hpp> // clsid for massspectrometer
#include <adportable/float.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/is_type.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>
#include <adcontrols/jcb2009_peakresult.hpp>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QItemDelegate>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QStandardItemModel>
#include <QSignalBlocker>
#include <QMenu>
#include <QPainter>
#include <QPair>
#include <QtPrintSupport/QPrinter>
#include <boost/format.hpp>
#include <boost/signals2.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/variant.hpp>
#include <boost/json.hpp>
#include <sstream>
#include <set>
#include <ratio>

namespace adwidgets {

    enum c_mspeaktable {
        c_mspeaktable_formula
        , c_mspeaktable_exact_mass
        , c_mspeaktable_mass
        , c_mspeaktable_mass_error
        , c_mspeaktable_delta_mass
        , c_mspeaktable_intensity
        , c_mspeaktable_relative_intensity
        , c_mspeaktable_mode
        , c_mspeaktable_time
        , c_mspeaktable_protocol
        , c_mspeaktable_mass_width
        , c_mspeaktable_time_width
        , c_mspeaktable_jcb2009_tR
        , c_mspeaktable_description
        , c_mspeaktable_index
        , c_mspeaktable_fcn
        , c_mspeaktable_is_reference
        , c_mspeaktable_num_columns
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


    struct index_finder {

        QModelIndex operator()( QAbstractItemModel& model, c_mspeaktable column, int index, int fcn ) {
            for ( int row = 0; model.rowCount(); ++row ) {
                if ( ( model.index( row, c_mspeaktable_index ).data( Qt::EditRole ).toInt() == index ) &&
                     ( model.index( row, c_mspeaktable_fcn ).data( Qt::EditRole ).toInt() == fcn ) ) {
                    return model.index( row, column );
                }
            }
            return {};
        }
    };


	using namespace adcontrols::metric;

    MSPeakTableDelegate::MSPeakTableDelegate(QObject *parent) : QItemDelegate( parent )
    {
    }

    void
    MSPeakTableDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
    {
        QStyleOptionViewItem op( option );
        op.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

        int fcn = index.model()->data( index.model()->index( index.row(), c_mspeaktable_fcn ) ).toInt();
        if ( fcn > 0 ) {
            size_t cid = ( fcn - 1 ) % (sizeof( colors )/sizeof( colors[ 0 ] ));
            painter->save();
            painter->fillRect( option.rect, colors[ cid ] );
            painter->restore();
        }
        switch( index.column() ) {
        case c_mspeaktable_time:
            op.displayAlignment = Qt::AlignRight | Qt::AlignHCenter;
            drawDisplay( painter, op, option.rect
                         , QString::number( scale_to_micro( index.data( Qt::EditRole ).toDouble() ), 'f', 5 ) );
            break;
        case c_mspeaktable_exact_mass:
            if ( ! index.model()->data( index.model()->index( index.row(), c_mspeaktable_formula ), Qt::EditRole ).toString().isEmpty() )
                drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'g', 7 ) );
            break;
        case c_mspeaktable_mass:
            drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'g', 7 ) );
            break;
        case c_mspeaktable_mass_error:
            if ( !index.model()->data( index.model()->index( index.row(), c_mspeaktable_formula ), Qt::EditRole ).toString().isEmpty() ) {
                double mDa = index.data().toDouble() * 1000;
                drawDisplay( painter, op, option.rect, QString::number( mDa, 'g', 5 ) );
            }
            break;
        case c_mspeaktable_intensity:
            if ( !index.model()->data( index.model()->index( index.row(), c_mspeaktable_intensity ), Qt::EditRole ).toString().isEmpty() )
                drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'g', 7 ) );
            // drawDisplay( painter, op, option.rect, (boost::format( "%.2lf" ) % (index.data( Qt::EditRole ).toDouble())).str().c_str() );
            break;
        case c_mspeaktable_relative_intensity:
            if ( !index.model()->data( index.model()->index( index.row(), c_mspeaktable_relative_intensity ), Qt::EditRole ).toString().isEmpty() )
                drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'f', 4 ) );
            break;
        case c_mspeaktable_formula:
            do {
                std::string formula = adcontrols::ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                DelegateHelper::render_html( painter, option, QString::fromStdString( formula ) );
            } while(0);
            break;
        case c_mspeaktable_jcb2009_tR:
            // if ( !index.model()->data( index.model()->index( index.row(), c_mspeaktable_jcb2009_tR ), Qt::EditRole ).isValid() )
            drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'f', 2 ) );
            break;
        case c_mspeaktable_description:
            DelegateHelper::render_html( painter, option, index.data().toString() );
            break;
        case c_mspeaktable_mass_width:
        case c_mspeaktable_time_width:
            drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'f', 2 ) ) ;
            break;
        case c_mspeaktable_mode:
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

    ///////////////////

    namespace detail {

        struct dataMayChanged : public boost::static_visitor< bool > {
            MSPeakTable * pThis_;
            dataMayChanged( MSPeakTable * table ) : pThis_( table ) {}
            bool operator()( std::weak_ptr< adcontrols::MassSpectrum >& wptr ) const {
                if ( auto ptr = wptr.lock() )
                    pThis_->setData( *ptr );
                return true;
            }
            bool operator()( std::weak_ptr< adcontrols::MSPeakInfo >& pkinfo ) const {
                if ( auto ptr = pkinfo.lock() )
                    pThis_->setPeakInfo( *ptr );
                return true;
            }
        };

        struct annotation_updator {

            bool operator () ( adcontrols::MassSpectrumPtr& ptr, int idx, int fcn, const std::string& formula ) {
                adcontrols::segment_wrapper<> segs( *ptr );
                if ( signed(segs.size()) > fcn ) {
                    auto& ms = segs[fcn];
                    if ( !formula.empty() ) {
                        ms.addAnnotation( { formula
                                , ms.mass( idx )
                                , ms.intensity( idx )
                                , idx
                                , 0
                                , adcontrols::annotation::dataFormula } );
                    } else {
                        ms.annotations().erase_if( [&](const auto& a ){
                            return a.index() == idx && a.dataFormat() == adcontrols::annotation::dataFormula; });
                    }
                    return true;
                }
                return false;
            }

        };

    }

    class MSPeakTable::impl {
    public:
        std::shared_ptr< QStandardItemModel > model_;
        std::shared_ptr< QItemDelegate > delegate_;

        impl() : model_( std::make_shared< QStandardItemModel >() )
               , delegate_( std::make_shared< MSPeakTableDelegate >() )
               , inProgress_( false ) {
        }

        std::shared_ptr< adcontrols::MassSpectrum > __getMassSpectrum() {
            if ( data_source_.which() == 1 ) {
                auto wptr = boost::get< std::weak_ptr< adcontrols::MassSpectrum > >( data_source_ );
                return wptr.lock();
            }
            return {};
        }

        std::shared_ptr< adcontrols::MSPeakInfo > __getMSPeakInfo() {
            if ( data_source_.which() == 0 ) {
                auto wptr = boost::get< std::weak_ptr< adcontrols::MSPeakInfo > >( data_source_ );
                return wptr.lock();
            }
            if ( auto ptr = pkinfo_.lock() ) {
                return ptr;
            }
            return {};
        }

        boost::variant< std::weak_ptr< adcontrols::MSPeakInfo >
                        , std::weak_ptr< adcontrols::MassSpectrum > > data_source_;

        std::weak_ptr< adcontrols::MSPeakInfo > pkinfo_;  // it is a pair of data_source_
        boost::signals2::signal< callback_t > callback_;
        bool inProgress_;
        std::weak_ptr< const adcontrols::MassSpectrometer > massSpectrometer_;
    };
}

namespace {

    template< int column, typename T >
    struct row_walker {
        std::set< T > values_;
        bool operator()( QAbstractItemModel * model ) {
            for ( int row = 0; row < model->rowCount(); ++row ) {
                auto v = model->index( row, column ).data();
                if ( v.canConvert< T >() ) {
                    values_.emplace( v.value<T>() );
                }
            }
            return values_.size() <= 1;
        }
    };
}

using namespace adwidgets;

MSPeakTable::~MSPeakTable()
{
    delete impl_;
}

MSPeakTable::MSPeakTable(QWidget *parent) : TableView(parent)
                                          , impl_( new impl() )
{
    this->setHorizontalHeader( new HtmlHeaderView );
    this->setModel( impl_->model_.get() );
	this->setItemDelegate( impl_->delegate_.get() );
    this->setSortingEnabled( true );
    this->setVerticalHeader( new QHeaderView(Qt::Vertical) );
    this->setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( showContextMenu( const QPoint& ) ) );
    connect( impl_->delegate_.get(), SIGNAL( valueChanged( const QModelIndex& ) ), this, SLOT( handleValueChanged( const QModelIndex& ) ) );
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
MSPeakTable::onUpdate( boost::any&& a )
{
    if ( adportable::a_type< adcontrols::MassSpectrumPtr >::is_a( a ) ) {
        // lockMassHandled on MainWindow invoke this method
        auto ptr = boost::any_cast< adcontrols::MassSpectrumPtr >( a );
        auto wptr = boost::get< std::weak_ptr< adcontrols::MassSpectrum > >( impl_->data_source_ );
        if ( wptr.lock() == ptr )
            updateData( *ptr );
        else
            setData( *ptr );
    } else if ( adportable::a_type< adcontrols::MSPeakInfoPtr >::is_a( a ) ) {
        if ( auto ptr = boost::any_cast< adcontrols::MSPeakInfoPtr >( a ) ) {
            setPeakInfo( *ptr );
        }
    } else if ( a.type() == typeid(int) ) {
        int id = boost::any_cast<int>( a );
        if ( id == 0 ) { // data may changed
            boost::apply_visitor( detail::dataMayChanged( this ), impl_->data_source_ );
        }
    }
}

void
MSPeakTable::dataMayChanged()
{
    boost::apply_visitor( detail::dataMayChanged( this ), impl_->data_source_ );
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
MSPeakTable::setContents( boost::any&& a )
{
    impl_->callback_.disconnect_all_slots();
    impl_->pkinfo_.reset();

    typedef std::pair< adcontrols::MassSpectrumPtr, adcontrols::MSPeakInfoPtr > spectrum_peakinfo_type;

    if ( adportable::a_type< spectrum_peakinfo_type >::is_a( a ) ) {

        auto pair = boost::any_cast< spectrum_peakinfo_type >( a );

        impl_->data_source_ = pair.first;
        impl_->pkinfo_ = pair.second;
        setPeakInfo( *pair.second );
        if ( pair.first ) {
            setAnnotations( pair.first );
        }
        return true;
    }

    if ( adportable::a_type< adcontrols::MSPeakInfoPtr >::is_a( a ) ) {
        std::weak_ptr< adcontrols::MSPeakInfo > wptr = boost::any_cast< adcontrols::MSPeakInfoPtr >( a );
        impl_->data_source_ = wptr;
        if ( auto ptr = wptr.lock() )
            setPeakInfo( *ptr );
        return true;
    }

    if ( adportable::a_type< adcontrols::MassSpectrumPtr >::is_a( a ) ) {
        std::weak_ptr< adcontrols::MassSpectrum > wptr = boost::any_cast< adcontrols::MassSpectrumPtr >( a );
        impl_->data_source_ = wptr;
        if ( auto ptr = wptr.lock() )
            setPeakInfo( *ptr );
        return true;
    }

    if ( adportable::a_type< adcontrols::TargetingPtr >::is_a( a ) ) {
        if ( auto tgt = boost::any_cast<adcontrols::TargetingPtr>(a) ) {
            setPeakInfo( *tgt );
        }
    }

    return false;
}

// this is for Spectrogram; selected centroid data review on MSPeakTable
void
MSPeakTable::setCentroidSpectrum( std::shared_ptr< adcontrols::MassSpectrum > ms, std::function< callback_t > callback )
{
    impl_->data_source_ = ms;
    impl_->callback_.disconnect_all_slots();

    if ( callback )
        impl_->callback_.connect( callback );

    setPeakInfo( *ms );
}

void
MSPeakTable::onInitialUpdate()
{
    QStandardItemModel& model = *impl_->model_;

    model.setColumnCount( c_mspeaktable_num_columns );

    model.setHeaderData( c_mspeaktable_time,        Qt::Horizontal, QObject::tr( "time(&mu;s)" ) );
    model.setHeaderData( c_mspeaktable_exact_mass,  Qt::Horizontal, QObject::tr( "Exact <i>m/z</i>" ) );
    model.setHeaderData( c_mspeaktable_mass,        Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
    model.setHeaderData( c_mspeaktable_mass_error,  Qt::Horizontal, QObject::tr( "error(mDa)" ) );
    model.setHeaderData( c_mspeaktable_delta_mass,  Qt::Horizontal, QObject::tr( "&delta;Da" ) );
    model.setHeaderData( c_mspeaktable_intensity,   Qt::Horizontal, QObject::tr( "Abundance" ) );
    model.setHeaderData( c_mspeaktable_relative_intensity,   Qt::Horizontal, QObject::tr( "R. A. (%)" ) );
    model.setHeaderData( c_mspeaktable_mode,        Qt::Horizontal, QObject::tr( "mode" ) );
    model.setHeaderData( c_mspeaktable_protocol,    Qt::Horizontal, QObject::tr( "protocol" ) );
    model.setHeaderData( c_mspeaktable_formula,     Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_mspeaktable_description, Qt::Horizontal, QObject::tr( "description" ) );

    model.setHeaderData( c_mspeaktable_mass_width,  Qt::Horizontal, QObject::tr( "width(mDa)" ) );
    model.setHeaderData( c_mspeaktable_time_width,  Qt::Horizontal, QObject::tr( "width(ns)" ) );
    model.setHeaderData( c_mspeaktable_jcb2009_tR,  Qt::Horizontal, QObject::tr( "t<sub>R</sub> s" ) );

    setColumnHidden( c_mspeaktable_is_reference, true );
    setColumnHidden( c_mspeaktable_index, true );
    setColumnHidden( c_mspeaktable_fcn, true );  // a.k.a. protocol id, internally used as an id

    setColumnHidden( c_mspeaktable_delta_mass, true );
    setColumnHidden( c_mspeaktable_jcb2009_tR, true );
    setColumnHidden( c_mspeaktable_relative_intensity, true );
    //horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents ); <-- performance killer
    //horizontalHeader()->setResizeMode( QHeaderView::Stretch );
}

void
MSPeakTable::setPeakInfo( const adcontrols::Targeting& targeting )
{
    QStandardItemModel& model = *impl_->model_;
    const auto& candidates = targeting.candidates();

    if ( candidates.empty() )
        return;

    size_t matchCount(0);

    for ( int row = 0; row < model.rowCount(); ++row ) {
        int idx = model.index( row, c_mspeaktable_index ).data( Qt::EditRole ).toInt();
        int fcn = model.index( row, c_mspeaktable_fcn ).data( Qt::EditRole ).toInt();

        auto it
            = std::find_if( candidates.begin(), candidates.end(), [=] ( const adcontrols::targeting::Candidate& c ){
                    return c.idx == uint32_t(idx) && c.fcn == uint32_t(fcn);  } );
        if ( it != candidates.end() ) {
            model.setData( model.index( row, c_mspeaktable_formula ), QString::fromStdString( it->formula ) );
            model.setData( model.index( row, c_mspeaktable_exact_mass ), it->exact_mass );
            model.setData( model.index( row, c_mspeaktable_mass_error ), it->mass - it->exact_mass );
            model.setData( model.index( row, c_mspeaktable_description ), QString::fromStdString( it->display_name ) );

            setRowHidden( row, false );
            matchCount++;
        }
    }
    if ( matchCount )
        hideRows();

    this->resizeColumnToContents( c_mspeaktable_formula );
}

void
MSPeakTable::setPeakInfo( const adcontrols::MSPeakInfo& info )
{
	QStandardItemModel& model = *impl_->model_;

    setUpdatesEnabled( false );

    model.setRowCount( 0 );
    model.setRowCount( static_cast< int >( info.total_size() ) );

    adcontrols::segment_wrapper< const adcontrols::MSPeakInfo > segs( info ); // adcontrols::MSPeakInfo

    const bool is_area = info.isAreaIntensity();

    double iMax(0);
    if ( !this->isColumnHidden( c_mspeaktable_relative_intensity ) ) {
        for ( auto& pkinfo: segs ) {
            auto it = std::max_element( pkinfo.begin(), pkinfo.end()
                                        , [&](const auto& a, const auto& b){
                                            return is_area ? (a.area() < b.area()) : (a.height() < b.height());
                                        });
            if ( it != pkinfo.end() ) {
                iMax = std::max( iMax, is_area ? it->area() : it->height() );
            }
        }
    }
    bool hasFormula( false );
    int row = 0;
    int fcn = 0;
    for ( auto& pkinfo: segs ) {

        QSignalBlocker block( impl_->model_.get() );

        int idx = 0;
        for ( auto& pk: pkinfo ) {
            if ( pk.is_reference() ) {
                model.setHeaderData( row,   Qt::Vertical, QString("*%1").arg( row + 1 ) );
            }
            model.setData( model.index( row, c_mspeaktable_is_reference ), pk.is_reference() ); // hidden
            model.setData( model.index( row, c_mspeaktable_fcn ), fcn ); // hidden
            model.setData( model.index( row, c_mspeaktable_index ), idx++ ); // hidden
            model.setData( model.index( row, c_mspeaktable_protocol ), fcn ); // protocol id (for display)
            model.setData( model.index( row, c_mspeaktable_time ), pk.time() );
            model.setData( model.index( row, c_mspeaktable_mass ), pk.mass() );

            auto abundance = is_area ? pk.area() : pk.height();
            model.setData( model.index( row, c_mspeaktable_intensity ), abundance );
            if ( !this->isColumnHidden( c_mspeaktable_relative_intensity ) ) {
                model.setData( model.index( row, c_mspeaktable_relative_intensity ), (abundance * 100) / iMax );
            }
            if ( auto mode = pk.mode() ) { // if peak has modified mode value
                model.setData( model.index( row, c_mspeaktable_mode ), *mode );
            } else {
                model.setData( model.index( row, c_mspeaktable_mode ), pkinfo.mode() );
            }
            if ( ! pk.formula().empty() ) {
                double mass = exactMass( pk.formula() );
                model.setData( model.index( row, c_mspeaktable_formula ), QString::fromStdString( pk.formula() ) );
                model.setData( model.index( row, c_mspeaktable_exact_mass ), mass );
                model.setData( model.index( row, c_mspeaktable_mass_error ), pk.mass() - mass );
                hasFormula = true;
            }
			model.setData( model.index( row, c_mspeaktable_description ), QString::fromStdString( pk.annotation() ) );
            model.setData( model.index( row, c_mspeaktable_mass_width ), pk.widthHH( false ) * std::milli::den );
            model.setData( model.index( row, c_mspeaktable_time_width ), pk.widthHH( true ) * std::nano::den );

            // setRowHidden( row, false );
            ++row;
        }
        ++fcn;
    }

    if ( hasFormula )
        hideRows();

    setColumnHidden( c_mspeaktable_mass_width, false );
    setColumnHidden( c_mspeaktable_time_width, false );
    setColumnHidden( c_mspeaktable_mode, row_walker< c_mspeaktable_mode, int >()( &model ) );
    setColumnHidden( c_mspeaktable_protocol, row_walker< c_mspeaktable_protocol, int >()( &model ) );

    resizeColumnToContents( c_mspeaktable_formula );
    setUpdatesEnabled( true );
    update();
}

void
MSPeakTable::setPeakInfo( const adcontrols::MassSpectrum& ms )
{
	QStandardItemModel& model = *impl_->model_;
    size_t total_size = 0;

    setUpdatesEnabled( false );

    setColumnHidden( c_mspeaktable_jcb2009_tR, true );
    std::set< int > protocols, modes;

    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    for( auto& t: segs )
        total_size += t.size();

    model.setRowCount( 0 );
    model.setRowCount( static_cast< int >( total_size ) );

    double maxIntensity = 0;
    for ( auto& ms: segs )
        maxIntensity = std::max( ms.maxIntensity(), maxIntensity );

    bool hasFormula( false );
    int row = 0;
    int fcn = 0;
    for ( auto& fms: segs ) {

        const adcontrols::annotations& annots = fms.annotations();
        QString protlabel;

        auto& descs = fms.getDescriptions();
        auto it = std::find_if( descs.begin(), descs.end()
                                , [] ( const adcontrols::description& d ){ return d.key<char>() == "acquire.protocol.label"; } );
        if ( it != descs.end() ) {
            protlabel = QString::fromStdWString( (boost::wformat( L"#%d %s" ) % fcn % it->text<wchar_t>() ).str() );
        } else {
            protlabel = QString::fromStdString( (boost::format( "#%1%" ) % fcn).str() );
        }

        for ( int idx = 0; idx < signed(fms.size()); ++idx ) {

            model.setData( model.index( row, c_mspeaktable_fcn ), fcn ); // protocol number
            model.setData( model.index( row, c_mspeaktable_index ), idx ); // hidden
            model.setData( model.index( row, c_mspeaktable_protocol ), protlabel );

            double mass = fms.mass( idx );
            model.setData( model.index( row, c_mspeaktable_time ), fms.time( idx ) );
            model.setData( model.index( row, c_mspeaktable_mass ), mass );
            model.setData( model.index( row, c_mspeaktable_intensity ), fms.intensity( idx ) );

            model.setData( model.index( row, c_mspeaktable_relative_intensity ), (fms.intensity( idx ) * 100 ) / maxIntensity );

            model.setData( model.index( row, c_mspeaktable_mode ), fms.mode() );
            modes.emplace( fms.mode() );

            model.setData( model.index( row, c_mspeaktable_formula ), QString() ); // clear formula
            model.setData( model.index( row, c_mspeaktable_exact_mass ), 0.0 );

            model.setData( model.index( row, c_mspeaktable_mass_width ), QVariant() ); // clear width
            model.setData( model.index( row, c_mspeaktable_time_width ), QVariant() ); // clear width
            model.setData( model.index( row, c_mspeaktable_jcb2009_tR ), QVariant() );

            auto it = std::find_if( annots.begin(), annots.end()
                                    , [idx] ( const adcontrols::annotation& a ){ return a.index() == idx; } );
            while ( it != annots.end() ) {
                if ( auto json = it->json() ) {
                    auto jv = adportable::json_helper::parse( json );
                    if ( adportable::json_helper::find( jv, "peaks" ) != boost::json::value{} ) {
                        auto t = boost::json::value_to< adcontrols::annotation::peak >( jv );
                        model.setData( model.index( row, c_mspeaktable_mode ), t.mode );
                    } else if ( adportable::json_helper::find( jv, "jcb2009_peakresult" ) != boost::json::value{} ) {
                        auto t = boost::json::value_to< adcontrols::jcb2009_peakresult >( jv );
                        model.setData( model.index( row, c_mspeaktable_jcb2009_tR ), t.tR() );
                        setColumnHidden( c_mspeaktable_jcb2009_tR, false );
                    } else if ( adportable::json_helper::find( jv, "refernce_molecule" ) != boost::json::value{} ) {
                        auto t = boost::json::value_to< adcontrols::annotation::reference_molecule >( jv );
                        model.setData( model.index( row, c_mspeaktable_formula), QString::fromStdString( t.formula_ + " " + t.adduct_ ) );
                    }
                }
                if ( it->dataFormat() == adcontrols::annotation::dataText ) {
                    model.setData( model.index( row, c_mspeaktable_description ), QString::fromStdString( it->text() ) );
                } else if ( it->dataFormat() == adcontrols::annotation::dataFormula ) {
                    model.setData( model.index( row, c_mspeaktable_formula ), QString::fromStdString( it->text() ) );
                    model.setData( model.index( row, c_mspeaktable_exact_mass ), exactMass( it->text() ) );
                    model.setData( model.index( row, c_mspeaktable_mass_error ), mass - exactMass( it->text() ) );
                    hasFormula = true;
                } else if ( it->dataFormat() == adcontrols::annotation::dataSmiles ) {
                    // todo, smiles, MOL, SVG
                }
				it = std::find_if( ++it, annots.end(), [=]( const adcontrols::annotation& a ){ return a.index() == idx; });
            }
            ++row;
        }
        ++fcn;
    }

    if ( hasFormula )
        hideRows();

    setColumnHidden( c_mspeaktable_mode, row_walker< c_mspeaktable_mode, int >()( &model ) );
    setColumnHidden( c_mspeaktable_protocol, row_walker< c_mspeaktable_protocol, int >()( &model ) );

    setColumnHidden( c_mspeaktable_mass_width, true );
    setColumnHidden( c_mspeaktable_time_width, true );

    //resizeColumnsToContents();
    //resizeRowsToContents();
    this->resizeColumnToContents( c_mspeaktable_formula );
    setUpdatesEnabled( true );
}

void
MSPeakTable::setAnnotations( std::shared_ptr< const adcontrols::MassSpectrum > ms )
{
    int proto(0);
    for ( auto& fms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *ms ) ) {
        const adcontrols::annotations& annots = fms.annotations();
        for ( auto anno: annots ) {
            if ( anno.index() >= 0 ) {
                auto index = index_finder()( *impl_->model_, c_mspeaktable_formula, anno.index(), proto );
                if ( index.isValid() ) {
                    if ( anno.dataFormat() == adcontrols::annotation::dataFormula ) {
                        impl_->model_->setData( index, QString::fromStdString( anno.text() ) );
                        double mass = exactMass( anno.text() );
                        impl_->model_->setData( impl_->model_->index( index.row(), c_mspeaktable_exact_mass ), mass );
                        if ( auto sp = impl_->massSpectrometer_.lock() ) {
                            int lap; double tof;
                            std::tie( lap, tof ) = sp->findLaps( mass, proto );
                            if ( lap > 0 ) {
                                impl_->model_->setData( impl_->model_->index( index.row(), c_mspeaktable_mode ), lap );
                            }
                        }
                    }
                } else if ( anno.dataFormat() == adcontrols::annotation::dataJSON ) {
                    auto jv = adportable::json_helper::parse( anno.json() );
                    if ( jv != boost::json::value{} ) {
                        boost::system::error_code ec;
                        if ( auto ptr = adportable::json_helper::find_pointer( jv, "/reference_molecule", ec ) ) {
                            auto refmol = boost::json::value_to< adcontrols::annotation::reference_molecule >( jv );
                            if ( not refmol.formula_.empty() ) {
                                impl_->model_->setData( index, QString::fromStdString( refmol.formula_ + " " + refmol.adduct_ ) );
                                impl_->model_->setData( impl_->model_->index( index.row(), c_mspeaktable_exact_mass ), refmol.exact_mass_ );
                            }
                        }
                    }
                }
            } else {
                // ADDEBUG() << "index: " << anno.index() << ", text: " << anno.text();
            }
        }
        ++proto;
    }
}

void
MSPeakTable::setData( const adcontrols::MassSpectrum& ms )
{
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( ms );

    setPeakInfo( ms );
    return;
}

void
MSPeakTable::updateData( const adcontrols::MassSpectrum& ms )
{
    // ============  Call from lockmass via onUpdate ===========
	QStandardItemModel& model = *impl_->model_;

    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    size_t total_size = 0;
    for( auto& t: segs )
        total_size += t.size();

    if ( int(total_size) != model.rowCount() ) {
        setPeakInfo( ms );
        return;
    }

    setUpdatesEnabled( false );

    for ( int row = 0; row < int(total_size); ++row ) {

        int idx = model.index( row, c_mspeaktable_index ).data( Qt::EditRole ).toInt();
        int fcn = model.index( row, c_mspeaktable_fcn ).data( Qt::EditRole ).toInt();

        if ( fcn < signed(segs.size()) ) {
            auto& fms = segs[ fcn ];

            double mass = fms.mass( idx );
            model.setData( model.index( row, c_mspeaktable_time ), fms.time( idx ) );
            model.setData( model.index( row, c_mspeaktable_mass ), mass );
            model.setData( model.index( row, c_mspeaktable_intensity ), fms.intensity( idx ) );
            model.setData( model.index( row, c_mspeaktable_mode ), fms.mode() );

            model.setData( model.index( row, c_mspeaktable_description ), QString{} );
            model.setData( model.index( row, c_mspeaktable_formula ), QString{} );
            model.setData( model.index( row, c_mspeaktable_exact_mass ), QVariant{} );

            const adcontrols::annotations& annots = fms.annotations();
            auto it = std::find_if( annots.begin()
                                    , annots.end(), [&]( const auto& a ){ return a.index() == idx; } );

            while ( it != annots.end() ) {
                if ( it->dataFormat() == adcontrols::annotation::dataText ) {
                    model.setData( model.index( row, c_mspeaktable_description ), QString::fromStdString( it->text() ) );
                } else if ( it->dataFormat() == adcontrols::annotation::dataFormula ) {
                    model.setData( model.index( row, c_mspeaktable_formula ), QString::fromStdString( it->text() ) );
                    model.setData( model.index( row, c_mspeaktable_exact_mass ), exactMass( it->text() ) );
                    model.setData( model.index( row, c_mspeaktable_mass_error ), mass - exactMass( it->text() ) );
                } else if ( it->dataFormat() == adcontrols::annotation::dataJSON ) {
                    auto jv = adportable::json_helper::parse( it->json() );
                    if ( auto ptr = adportable::json_helper::if_contains( jv, "refernce_molecule" ) ) {
                        auto ref = boost::json::value_to< adcontrols::annotation::reference_molecule >( jv );
                        model.setData( model.index( row, c_mspeaktable_formula ), QString::fromStdString( ref.formula_ + " " + ref.adduct_ ) );
                        model.setData( model.index( row, c_mspeaktable_exact_mass ), ref.exact_mass_ );
                        model.setData( model.index( row, c_mspeaktable_mass_error ), mass - ref.exact_mass_ );
                    } else {
                        ADDEBUG() << "\t\treference_molecule cannot be found!";
                    }
                }
				it = std::find_if( ++it, annots.end(), [&]( const auto& a ){ return a.index() == idx; });
            }
        }
    }
    setUpdatesEnabled( true );
}

void
MSPeakTable::currentChanged( const QModelIndex& index, const QModelIndex& prev )
{
    QStandardItemModel& model = *impl_->model_;
    (void)prev;

    scrollTo( index, QAbstractItemView::EnsureVisible );

	int row = index.row();

    if ( isRowHidden( row ) )
        setRowHidden( row, false );

    int idx = model.index( row, c_mspeaktable_index ).data( Qt::EditRole ).toInt();
    int fcn = model.index( row, c_mspeaktable_fcn ).data( Qt::EditRole ).toInt();

    setUpdatesEnabled( false );
    double mass = model.index( row, c_mspeaktable_mass ).data( Qt::EditRole ).toDouble();
    for ( int r = 0; r < model.rowCount(); ++r ) {
        double d = std::abs( model.index( r, c_mspeaktable_mass ).data( Qt::EditRole ).toDouble() - mass );
        model.setData( model.index( r, c_mspeaktable_delta_mass ), d ); // int( d + 0.7 ) );
    }
    setUpdatesEnabled( true );

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
MSPeakTable::handleZoomedOnSpectrum( const QRectF& rc, int axis )
{
    QStandardItemModel& model = *impl_->model_;

    bool isTimeAxis = axis == adcontrols::hor_axis_time;

    if ( impl_->data_source_.which() == 1 ) {
        auto wptr = boost::get< std::weak_ptr< adcontrols::MassSpectrum > >( impl_->data_source_ );
        if ( auto ptr = wptr.lock() ) {
            std::pair<int, int> bp = adcontrols::segments_helper::base_peak_index( *ptr, rc.left(), rc.right(), isTimeAxis ); // index,fcn

            if ( bp.first >= 0 && bp.second >= 0 ) {
                do {
                    // ---> change rel. intensity
                    setUpdatesEnabled( false );
                    double base_height = adcontrols::segments_helper::get_intensity( *ptr, bp );
                    for ( int row = 0; row < model.rowCount(); ++row ) {
                        model.setData( model.index( row, c_mspeaktable_relative_intensity )
                                       , model.index( row, c_mspeaktable_intensity ).data().toDouble() * 100 / base_height );
                    }
                    setUpdatesEnabled( true );
                    // <--- end rel. intensity
                } while ( 0 );

                do {
                    for ( int row = 0; row < model.rowCount(); ++row ) {
                        if ( model.index( row, c_mspeaktable_index ).data( Qt::EditRole ).toInt() == bp.first
                             && model.index( row, c_mspeaktable_fcn ).data( Qt::EditRole ).toInt() == bp.second ) {

                            QModelIndex index = model.index( row, isTimeAxis ? c_mspeaktable_time : c_mspeaktable_mass );
                            setCurrentIndex( index );
                            scrollTo( index, QAbstractItemView::EnsureVisible );
                            break;
                        }
                    }
                } while ( 0 );
            }
        }
    }
}

// void
// MSPeakTable::handleCopyToClipboard()
// {
//     ADDEBUG() << "######## handleCopyToClipboard() ##############";
//     TableView::copyToClipboard( false );
// }

void
MSPeakTable::showContextMenu( const QPoint& pt )
{
    QStandardItemModel& model = *impl_->model_;
    QModelIndex index = currentIndex();

	if ( index.isValid() ) {

        QMenu menu;

        menu.addAction( "Hide", this, SLOT( hideRows() ) );
        menu.addAction( "Show all", this, SLOT( showRows() ) );

        QString text = QString("Rescale @ protocol %1").arg( model.data( model.index( index.row(), c_mspeaktable_protocol ) ).toString() );
        menu.addAction( text, this, [&](){
                int proto = model.data( model.index( index.row(), c_mspeaktable_fcn ) ).toInt();
                emit rescaleY( proto );
            });

        QModelIndexList list = selectionModel()->selectedIndexes();
        if ( list.size() < 1 )
            return;

        std::set< int > rows;
        for ( auto index: list )
            rows.insert( index.row() ); // make unique row list

        //----------- gather references ----------
        QString formulae = "Lock mass with ";
        QVector< QPair<int, int> > refs;

        for ( int row: rows ) {

            QString formula = model.data( model.index( row, c_mspeaktable_formula ) ).toString();

            if ( ! formula.isEmpty() ) {
                formulae += QString( (refs.isEmpty() ? "%1" : ", %1") ).arg( formula );
                int idx = model.data( model.index( row, c_mspeaktable_index ) ).toInt();
                int fcn = model.data( model.index( row, c_mspeaktable_fcn ) ).toInt();

                refs.push_back( QPair<int,int>( idx, fcn ) );
            }
        }

        //------------ lock mass
        if ( impl_->callback_.empty() )
            menu.addAction( formulae, [this,refs](){ emit triggerLockMass( refs ); } );
        else
            menu.addAction( formulae, [this,refs](){ impl_->callback_( lockmass_triggered, refs ); } ); // for SpectrogramWnd

        //------------ scan law estimation
        menu.addAction( "Estimate scan law...", [this,refs](){ emit estimateScanLaw( refs ); }  );

        //------------ Copy assigned
        menu.addAction( tr("Copy assigned peaks to clipboard"), this, SLOT( handleCopyAssignedPeaks() ) );

        //-- add dataprocessor dependent menu --
        addContextMenu( menu, pt, this, list );
        //if ( impl_->data_source_.which() == 1 ) {
        //    auto wptr = boost::get< std::weak_ptr< adcontrols::MassSpectrum > >( impl_->data_source_ );
        //    addContextMenu( menu, pt, wptr.lock() );
        //    menu.addSeparator();
        //}

        //-- add base TableView's menu --
        addActionsToContextMenu( menu, pt );

        menu.exec( this->mapToGlobal( pt ) );
    }
}

void
MSPeakTable::handleValueChanged( const QModelIndex& index )
{
    if ( impl_->inProgress_ )
        return;
    if ( index.column() == c_mspeaktable_formula ) {
        formulaChanged( index );
    } else if ( index.column() == c_mspeaktable_description ) {
        descriptionChanged( index );
    } else if ( index.column() == c_mspeaktable_mode ) {
        modeChanged( index );
    }
}

void
MSPeakTable::formulaChanged( const QModelIndex& index )
{
	QStandardItemModel& model = *impl_->model_;

    if ( index.column() == c_mspeaktable_formula ) {

        int fcn = model.index( index.row(), c_mspeaktable_fcn ).data( Qt::EditRole ).toInt();
        int idx = model.index( index.row(), c_mspeaktable_index ).data( Qt::EditRole ).toInt();

        std::string formula = index.data( Qt::EditRole ).toString().toStdString();
        double mass = model.index( index.row(), c_mspeaktable_mass ).data( Qt::EditRole ).toDouble();

        model.setData( model.index( index.row(), c_mspeaktable_exact_mass ), exactMass( formula ) );
        model.setData( model.index( index.row(), c_mspeaktable_mass_error ), mass - exactMass( formula ) );

        if ( impl_->data_source_.which() == 0 ) {
            auto wptr = boost::get< std::weak_ptr< adcontrols::MSPeakInfo > >( impl_->data_source_ );
            if ( auto ptr = wptr.lock() ) {
                adcontrols::segment_wrapper< adcontrols::MSPeakInfo > segs( *ptr );
                auto it = segs[ fcn ].begin() + idx;
                it->formula( formula );
            }
        } else {
            auto wptr = boost::get< std::weak_ptr< adcontrols::MassSpectrum > >( impl_->data_source_ );
            if ( auto ptr = wptr.lock() ) {
                if ( detail::annotation_updator()( ptr, idx, fcn, formula ) ) {
                    if ( !impl_->callback_.empty() )
                        impl_->callback_( formula_changed, QVector< QPair<int,int> >() );
                    else
                        emit formulaChanged( idx, fcn );
                }
            }
        }
    }
}

void
MSPeakTable::descriptionChanged( const QModelIndex& index )
{
    QStandardItemModel& model = *impl_->model_;

    if ( index.column() == c_mspeaktable_description ) {

        int fcn = model.index( index.row(), c_mspeaktable_fcn ).data( Qt::EditRole ).toInt();
        int idx = model.index( index.row(), c_mspeaktable_index ).data( Qt::EditRole ).toInt();

        std::wstring description = index.data( Qt::EditRole ).toString().toStdWString();

        if ( impl_->data_source_.which() == 0 ) {
            auto wptr = boost::get< std::weak_ptr< adcontrols::MSPeakInfo > >( impl_->data_source_ );
            if ( auto ptr = wptr.lock() ) {
                adcontrols::segment_wrapper< adcontrols::MSPeakInfo > segs( *ptr );
                auto it = segs[ fcn ].begin() + idx;
                it->annotation( description );
            }
        } else {
            auto wptr = boost::get< std::weak_ptr< adcontrols::MassSpectrum > >( impl_->data_source_ );
            if ( auto ptr = wptr.lock() ) {
                adcontrols::segment_wrapper<> segs( *ptr );
                if ( signed(segs.size()) > fcn ) {
                    auto& ms = segs[fcn];
                    if ( description.empty() )
                        ms.annotations().erase_if( [&](const auto& a){ return a.index() == idx && a.dataFormat() == adcontrols::annotation::dataText; });
                    else
                        ms.addAnnotation({ description
                                , ms.mass( idx )
                                , ms.intensity( idx )
                                , idx
                                , 0
                                , adcontrols::annotation::dataText } );
                    emit formulaChanged( idx, fcn );
                }
            }
        }
    }
}

void
MSPeakTable::modeChanged( const QModelIndex& index )
{
	QStandardItemModel& model = *impl_->model_;

    if ( index.column() != c_mspeaktable_mode )
        return;

    if ( auto sp = impl_->massSpectrometer_.lock() ) {
        int fcn = model.index( index.row(), c_mspeaktable_fcn ).data( Qt::EditRole ).toInt();
        int idx = model.index( index.row(), c_mspeaktable_index ).data( Qt::EditRole ).toInt();
        int mode = index.data( Qt::EditRole ).toInt();

        double time = model.index( index.row(), c_mspeaktable_time ).data( Qt::EditRole ).toDouble();
        double mass = sp->assignMass( time, mode );
        model.setData( model.index( index.row(), c_mspeaktable_mass ), mass );

        if ( auto pkInfo = impl_->__getMSPeakInfo() ) {
            adcontrols::segment_wrapper< adcontrols::MSPeakInfo > fpks( *pkInfo );
            if ( fpks[ fcn ].size() > idx ) {
                auto pk = fpks[ fcn ].begin() + idx;
                pk->set_mass( sp->assignMass( pk->time(), mode )
                              , sp->assignMass( pk->hh_left_time(), mode )
                              , sp->assignMass( pk->hh_right_time(), mode ) );
                if ( mode != fpks[ fcn ].mode() ) {
                    pk->set_mode( mode ); // mode can be boost::none
                }
            }
        }
        emit modeChanged( idx, fcn, mode );
    }
}


double
MSPeakTable::exactMass( std::string formula )
{
    if ( formula.empty() )
        return 0;
    // auto neutral = adcontrols::ChemicalFormula::neutralize( formula );
    // ADDEBUG() << "exactMass(" << formula << ") neutral: " << neutral;
    // ADDEBUG() << adcontrols::ChemicalFormula().getMonoIsotopicMass( adcontrols::ChemicalFormula::split( formula ) );
    // ADDEBUG() << adcontrols::ChemicalFormula().getMonoIsotopicMass( adcontrols::ChemicalFormula::split( neutral.first ) );
    return adcontrols::ChemicalFormula().getMonoIsotopicMass( adcontrols::ChemicalFormula::split( formula ) ).first;
}

bool
MSPeakTable::getMSPeak( adcontrols::MSPeak& peak, int row ) const
{
	QStandardItemModel& model = *impl_->model_;

    peak.time( model.index( row, c_mspeaktable_time ).data( Qt::EditRole ).toDouble() );
    peak.mass( model.index( row, c_mspeaktable_mass ).data( Qt::EditRole ).toDouble() );
    peak.mode( model.index( row, c_mspeaktable_mode ).data( Qt::EditRole ).toInt() );

    peak.fcn( model.index( row, c_mspeaktable_fcn ).data( Qt::EditRole ).toInt() );
    //peak.width( double, bool isTime = false );
    //peak.exit_delay( double );
    //peak.flight_length( double );
    peak.formula( model.index( row, c_mspeaktable_formula ).data( Qt::EditRole ).toString().toStdString() );
    peak.description( model.index( row, c_mspeaktable_formula ).data( Qt::EditRole ).toString().toStdString() );
    peak.spectrumIndex( model.index( row, c_mspeaktable_index ).data( Qt::EditRole ).toInt() );

    return true;
}

std::shared_ptr< adcontrols::MSPeaks >
MSPeakTable::getSelectedPeaks( GETPEAKOPTS opt ) const
{
    auto pks = std::make_shared< adcontrols::MSPeaks>();
    if ( getMSPeaks( *pks, opt ) )
        return pks;
    return {};
}

bool
MSPeakTable::getMSPeaks( adcontrols::MSPeaks& peaks, GETPEAKOPTS opt ) const
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

void
MSPeakTable::setMSPeak( const adcontrols::MSPeak& pk ) // set peak via spectrumIndex
{
    for ( int row = 0; impl_->model_->rowCount(); ++row ) {
        auto index = index_finder()( *impl_->model_, c_mspeaktable_mode, pk.spectrumIndex(), pk.fcn() );
        if ( index != QModelIndex() ) {
            impl_->model_->setData( index, pk.mode() );
            impl_->model_->setData( impl_->model_->index( index.row(), c_mspeaktable_mass ), pk.mass() );
            break;
        }
    }
}

int
MSPeakTable::findColumn( const QString& name ) const
{
    int nColumn = impl_->model_->columnCount();
    for ( int col = 0; col < nColumn; ++col ) {
        if ( impl_->model_->headerData( col, Qt::Horizontal, Qt::EditRole ).toString() == name )
            return col;
    }
    return -1;
}

void
MSPeakTable::handleCopyAssignedPeaks()
{
    QStandardItemModel& model = *impl_->model_;

    QString selected_text;

    // copy header --->
    for ( int col = 0; col < model.columnCount(); ++col ) {
        if ( !isColumnHidden( col ) ) {
            selected_text.append( model.headerData( col, Qt::Horizontal ).toString() );
            selected_text.append( '\t' );
        }
    }
    selected_text.append( '\n' );
    // <-------------

    for ( int row = 0; row < model.rowCount(); ++row ) {
        auto formula = model.data( model.index( row, c_mspeaktable_formula ) ).toString();
        if ( !formula.isEmpty() ) {

            for ( int col = 0; col < model.columnCount(); ++col ) {
                selected_text.append( model.index( row, col ).data( Qt::EditRole ).toString() );
                if ( col != model.columnCount() - 1 )
                    selected_text.append( '\t' );
            }
            selected_text.append( '\n' );
        }
    }

    QApplication::clipboard()->setText( selected_text );

}

void
MSPeakTable::hideRows()
{
    QStandardItemModel& model = *impl_->model_;
    for ( int row = 0; row < model.rowCount(); ++row ) {
        bool hide = model.data( model.index( row, c_mspeaktable_formula ) ).toString().isEmpty();
        setRowHidden( row, hide );
    }

}

void
MSPeakTable::showRows()
{
    QStandardItemModel& model = *impl_->model_;
    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( isRowHidden( row ) )
            setRowHidden( row, false );
    }
}

void
MSPeakTable::handlePrint( QPrinter& printer, QPainter& painter )
{
    const QStandardItemModel& model = *(impl_->model_);
    printer.newPage();
    auto pageRect = printer.pageLayout().paintRectPixels(printer.resolution());
	const QRect rect( pageRect.x() + pageRect.width() * 0.05
                      , pageRect.y() + pageRect.height() * 0.05
                      , pageRect.width() * 0.9, pageRect.height() * 0.8 );

    const int rows = model.rowCount();
    const int cols = model.columnCount();

    grid_render render( rect );

    for ( int col = 0; col < cols; ++col ) {
        double width = 0;
        switch( col ) {
        case c_mspeaktable_formula:              width = rect.width() / 180 * 40; break;
        case c_mspeaktable_exact_mass:           width = rect.width() / 180 * 30; break;
        case c_mspeaktable_mass:                 width = rect.width() / 180 * 30; break;
        case c_mspeaktable_mass_error:           width = rect.width() / 180 * 24; break;
        case c_mspeaktable_intensity:            width = rect.width() / 180 * 24; break;
        case c_mspeaktable_relative_intensity:   width = rect.width() / 180 * 24; break;
        case c_mspeaktable_time:                 width = rect.width() / 180 * 18; break;
        case c_mspeaktable_description:          width = rect.width() / 180 * 30; break;
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

        if ( isRowHidden( row ) )
            continue;

        QString formula = model.index( row, c_mspeaktable_formula ).data( Qt::EditRole ).toString();

        for ( int col = 0; col < cols; ++col ) {
            auto data = model.index( row, col ).data( Qt::EditRole );
            QString text;
            switch( col ) {
            case c_mspeaktable_formula:
                text = QString::fromStdString( adcontrols::ChemicalFormula::formatFormulae( data.toString().toStdString() ) );
                render( painter, col, text, "left" );
                break;
            case c_mspeaktable_exact_mass:
                text = formula.isEmpty() ? QString() : QString::number( data.toDouble(), 'g', 7 );
                render( painter, col, text );
                break;
            case c_mspeaktable_mass:
                text = QString::number( data.toDouble(), 'g', 7 );
                render( painter, col, text );
                break;
            case c_mspeaktable_mass_error:
                text = QString::number( ( data.toDouble() * std::milli::den ), 'g', 7 );
                render( painter, col, text );
                break;
            case c_mspeaktable_intensity:
                text = QString::number( data.toDouble(), 'g', 1 );
                render( painter, col, text );
                break;
            case c_mspeaktable_relative_intensity:
                text = QString::number( data.toDouble(), 'g', 4 );
                render( painter, col, text );
                break;
            case c_mspeaktable_time:
                text = QString::number( data.toDouble(), 'g', 4 );
                render( painter, col, text );
                break;
            case c_mspeaktable_description:
                text = data.toString();
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
MSPeakTable::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > sp )
{
    impl_->massSpectrometer_ = sp;
    if ( sp ) {
        if ( sp->massSpectrometerClsid() == qtplatz::infitof::iids::uuid_massspectrometer ) {
        }
    }
}

std::shared_ptr< const adcontrols::MassSpectrometer >
MSPeakTable::massSpectrometer() const
{
    return impl_->massSpectrometer_.lock();
}
