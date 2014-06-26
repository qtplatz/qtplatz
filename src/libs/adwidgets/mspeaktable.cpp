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

#include "mspeaktable.hpp"
#include <adcontrols/annotations.hpp>
#include <adcontrols/annotation.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adportable/float.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/is_type.hpp>
#include <qtwrapper/font.hpp>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QStandardItemModel>
#include <QDebug>
#include <QMenu>
#include <QPair>
#include <boost/format.hpp>
#include <sstream>
#include <set>

namespace adwidgets {

    enum {
        c_mspeaktable_formula
        , c_mspeaktable_mass
        , c_mspeaktable_mass_error
        , c_mspeaktable_delta_mass
        , c_mspeaktable_intensity
        , c_mspeaktable_relative_intensity
        , c_mspeaktable_mode
        , c_mspeaktable_time
        , c_mspeaktable_protocol
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
        QStyleOptionViewItem op( option );
        op.displayAlignment = Qt::AlignRight | Qt::AlignHCenter;

        switch( index.column() ) {
        case c_mspeaktable_time:
            op.displayAlignment = Qt::AlignRight | Qt::AlignHCenter;
            drawDisplay( painter, op, option.rect
                , (boost::format( "%.5lf" ) % scale_to_micro( index.data( Qt::EditRole ).toDouble() )).str().c_str() );
            break;
        case c_mspeaktable_mass:
            drawDisplay( painter, op, option.rect, (boost::format( "%.7lf" ) % index.data( Qt::EditRole ).toDouble()).str().c_str() );
            break;
        case c_mspeaktable_mass_error:
            if ( !index.model()->data( index.model()->index( index.row(), c_mspeaktable_formula ), Qt::EditRole ).toString().isEmpty() )
                drawDisplay( painter, op, option.rect, (boost::format( "%.7g" ) % (index.data( Qt::EditRole ).toDouble() * 1000)).str().c_str() );
            break;
        case c_mspeaktable_intensity:
            if ( !index.model()->data( index.model()->index( index.row(), c_mspeaktable_intensity ), Qt::EditRole ).toString().isEmpty() )
                drawDisplay( painter, op, option.rect, (boost::format( "%.2lf" ) % (index.data( Qt::EditRole ).toDouble())).str().c_str() );
            break;
        case c_mspeaktable_relative_intensity:
            if ( !index.model()->data( index.model()->index( index.row(), c_mspeaktable_relative_intensity ), Qt::EditRole ).toString().isEmpty() )
                drawDisplay( painter, op, option.rect, (boost::format( "%.2lf" ) % (index.data( Qt::EditRole ).toDouble())).str().c_str() );
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

    ///////////////////

    namespace detail {

        template<class T> struct lock_weak_pointer : public boost::static_visitor< std::shared_ptr<T> > {
            std::shared_ptr< T > operator ()( std::weak_ptr<T>& wptr ) const {
                return wptr.lock();
            }
        };

        struct dataMayChanged : public boost::static_visitor< bool > {
            MSPeakTable * pThis_;
            dataMayChanged( MSPeakTable * table ) : pThis_( table ) {}
            bool operator()( std::weak_ptr< adcontrols::MassSpectrum >& wptr ) const {
                if ( auto ptr = wptr.lock() )
                    pThis_->dataChanged( *ptr );
                return true;
            }
            bool operator()( std::weak_ptr< adcontrols::MSPeakInfo >& ) const {
                return false; // do nothing
            }
        };

    }
}

using namespace adwidgets;

MSPeakTable::MSPeakTable(QWidget *parent) : QTableView(parent)
                                          , model_( std::make_shared< QStandardItemModel >() )
										  , delegate_( std::make_shared< MSPeakTableDelegate >() )
                                          , inProgress_( false )
{
    this->setModel( model_.get() );
	this->setItemDelegate( delegate_.get() );
    this->setSortingEnabled( true );
    this->verticalHeader()->setDefaultSectionSize( 18 );
    this->setContextMenuPolicy( Qt::CustomContextMenu );

    QFont font;
	this->setFont( qtwrapper::font::setFont( font, qtwrapper::fontSizeSmall, qtwrapper::fontTableBody ) );

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
MSPeakTable::onUpdate( boost::any& a )
{
    if ( adportable::a_type< adcontrols::MassSpectrumPtr >::is_a( a ) ) {
        // lockMassHandled on MainWindow invoke this method

        auto ptr = boost::any_cast< adcontrols::MassSpectrumPtr >( a );
        auto wptr = boost::get< std::weak_ptr< adcontrols::MassSpectrum > >( data_source_ );
        if ( wptr.lock() == ptr )
            dataChanged( *ptr );                        

    } else if ( a.type() == typeid(int) ) {
        // dataMayChanged on MainWindow invoke this method over applyCalibration()

        int id = boost::any_cast<int>( a );

        if ( id == 0 ) { // data may changed
            boost::apply_visitor( detail::dataMayChanged( this ), data_source_ );
        }
    }
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
        std::weak_ptr< adcontrols::MSPeakInfo > wptr = boost::any_cast< adcontrols::MSPeakInfoPtr >( a );
        data_source_ = wptr;
        if ( auto ptr = wptr.lock() )
            setPeakInfo( *ptr );            
        return true;
    }

    if ( adportable::a_type< adcontrols::MassSpectrumPtr >::is_a( a ) ) {
        std::weak_ptr< adcontrols::MassSpectrum > wptr = boost::any_cast< adcontrols::MassSpectrumPtr >( a );
        data_source_ = wptr;
        if ( auto ptr = wptr.lock() )
            setPeakInfo( *ptr );            
        return true;
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
    model.setHeaderData( c_mspeaktable_intensity,   Qt::Horizontal, QObject::tr( "Abandance" ) );
    model.setHeaderData( c_mspeaktable_relative_intensity,   Qt::Horizontal, QObject::tr( "R. A." ) );
    model.setHeaderData( c_mspeaktable_mode,        Qt::Horizontal, QObject::tr( "mode" ) );
    model.setHeaderData( c_mspeaktable_protocol,    Qt::Horizontal, QObject::tr( "protocol" ) );
    model.setHeaderData( c_mspeaktable_formula,     Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_mspeaktable_description, Qt::Horizontal, QObject::tr( "description" ) );

    setColumnHidden( c_mspeaktable_index, true );
    setColumnHidden( c_mspeaktable_fcn, true );  // a.k.a. protocol id, internally used as an id

    //horizontalHeader()->setResizeMode( QHeaderView::Stretch );
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

            model.setData( model.index( row, c_mspeaktable_protocol ), fcn ); // protocol id (for display)
            
            model.setData( model.index( row, c_mspeaktable_time ), pk.time() );
            model.setData( model.index( row, c_mspeaktable_mass ), pk.mass() );
            model.setData( model.index( row, c_mspeaktable_intensity ), pk.area() );
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
}

void
MSPeakTable::setPeakInfo( const adcontrols::MassSpectrum& ms )
{
	QStandardItemModel& model = *model_;
    size_t total_size = 0;

    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    for( auto& t: segs )
        total_size += t.size();

    model.setRowCount( static_cast< int >( total_size ) );
	
    int row = 0;
    int fcn = 0;
    for ( auto& fms: segs ) {

        const adcontrols::annotations& annots = fms.get_annotations();
        QString protlabel;

        auto& descs = fms.getDescriptions();
        auto it = std::find_if( descs.begin(), descs.end(), [] ( const adcontrols::Description& d ){ return d.key() == L"acquire.protocol.label"; } );
        if ( it != descs.end() ) {
            protlabel = QString::fromStdWString( (boost::wformat( L"#%d %s" ) % fcn % it->text() ).str() );
        }
        else {
            protlabel = QString::fromStdString( (boost::format( "#%1%" ) % fcn).str() );
        }

        for ( int idx = 0; idx < signed(fms.size()); ++idx ) {

            model.setData( model.index( row, c_mspeaktable_fcn ), fcn ); // protocol number
            model.setData( model.index( row, c_mspeaktable_index ), idx ); // hidden
            model.setData( model.index( row, c_mspeaktable_protocol ), protlabel );

            double mass = fms.getMass( idx );
            model.setData( model.index( row, c_mspeaktable_time ), fms.getTime( idx ) );
            model.setData( model.index( row, c_mspeaktable_mass ), mass );
            model.setData( model.index( row, c_mspeaktable_intensity ), fms.getIntensity( idx ) );

            model.setData( model.index( row, c_mspeaktable_mode ), fms.mode() );

            model.setData( model.index( row, c_mspeaktable_formula ), QString() ); // clear formula

            auto it = std::find_if( annots.begin(), annots.end(), [=]( const adcontrols::annotation& a ){ return a.index() == idx; } );
            while ( it != annots.end() ) {
                if ( it->dataFormat() == adcontrols::annotation::dataText ) {
                    model.setData( model.index( row, c_mspeaktable_description ), QString::fromStdString( it->text() ) );                    
                } else if ( it->dataFormat() == adcontrols::annotation::dataFormula ) {
                    model.setData( model.index( row, c_mspeaktable_formula ), QString::fromStdString( it->text() ) );
                    model.setData( model.index( row, c_mspeaktable_mass_error ), mass - exactMass( it->text() ) );
                } 
                // todo: smiles, MOL
				it = std::find_if( ++it, annots.end(), [=]( const adcontrols::annotation& a ){ return a.index() == idx; });
            }
            ++row;
        }
        ++fcn;
    }

    resizeColumnsToContents();
    resizeRowsToContents();
}

void
MSPeakTable::dataChanged( const adcontrols::MassSpectrum& ms )
{
	QStandardItemModel& model = *model_;

    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    size_t total_size = 0;
    for( auto& t: segs )
        total_size += t.size();

    if ( total_size != model.rowCount() ) {
        setPeakInfo( ms );
        return;
    }

    for ( int row = 0; row < total_size; ++row ) {

        int idx = model.index( row, c_mspeaktable_index ).data( Qt::EditRole ).toInt();
        int fcn = model.index( row, c_mspeaktable_fcn ).data( Qt::EditRole ).toInt();
        if ( fcn < signed(segs.size()) ) {
            auto& fms = segs[ fcn ];

            double mass = fms.getMass( idx );
            model.setData( model.index( row, c_mspeaktable_time ), fms.getTime( idx ) );
            model.setData( model.index( row, c_mspeaktable_mass ), mass );
            model.setData( model.index( row, c_mspeaktable_intensity ), fms.getIntensity( idx ) );
            model.setData( model.index( row, c_mspeaktable_mode ), fms.mode() );

            model.setData( model.index( row, c_mspeaktable_description ), QString() );
            model.setData( model.index( row, c_mspeaktable_formula ), QString() );

            const adcontrols::annotations& annots = fms.get_annotations();
            auto it = std::find_if( annots.begin(), annots.end(), [=]( const adcontrols::annotation& a ){ return a.index() == idx; } );
            while ( it != annots.end() ) {
                if ( it->dataFormat() == adcontrols::annotation::dataText ) {
                    model.setData( model.index( row, c_mspeaktable_description ), QString::fromStdString( it->text() ) );                    
                } else if ( it->dataFormat() == adcontrols::annotation::dataFormula ) {
                    model.setData( model.index( row, c_mspeaktable_formula ), QString::fromStdString( it->text() ) );
                    model.setData( model.index( row, c_mspeaktable_mass_error ), mass - exactMass( it->text() ) );
                } 
				it = std::find_if( ++it, annots.end(), [=]( const adcontrols::annotation& a ){ return a.index() == idx; });
            }
        }
    }
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
MSPeakTable::handle_zoomed( const QRectF& rc )
{
    if ( data_source_.which() == 1 ) {
        auto wptr = boost::get< std::weak_ptr< adcontrols::MassSpectrum > >( data_source_ );
        if ( auto ptr = wptr.lock() ) {
            std::pair<int, int> bp = adcontrols::segments_helper::base_peak_index( *ptr, rc.left(), rc.right() );
            if ( bp.first >= 0 && bp.second >= 0 ) {
                // ---> change rel. intensity
                double base_height = adcontrols::segments_helper::get_intensity( *ptr, bp );
                for ( int row = 0; row < model_->rowCount(); ++row )
                    model_->setData( model_->index( row, c_mspeaktable_relative_intensity ), model_->index( row, c_mspeaktable_intensity ).data().toDouble() * 100 / base_height );
                // <--- end rel. intensity

                for ( int row = 0; row < model_->rowCount(); ++row ) {
                    if ( model_->index( row, c_mspeaktable_index ).data( Qt::EditRole ).toInt() == bp.first 
                         && model_->index( row, c_mspeaktable_fcn ).data( Qt::EditRole ).toInt() == bp.second ) {
                        
                        setCurrentIndex( model_->index( row, c_mspeaktable_mass ) );
                        scrollTo( model_->index( row, c_mspeaktable_mass ) );
                        break;
                    }
                }
            }
        }
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
    QModelIndex index = currentIndex();

	if ( index.isValid() ) {
        std::vector< QAction * > actions;
        QMenu menu;
        
        QModelIndexList list = selectionModel()->selectedIndexes();
        if ( list.size() < 1 )
            return;

        std::set< int > rows;
        for ( auto index: list )
            rows.insert( index.row() ); // make unique row list

        std::ostringstream o;
        o << "Lock mass with ";

        QVector< QPair<int, int> > refs;

        for ( int row: rows ) {

            QString formula = model_->data( model_->index( row, c_mspeaktable_formula ) ).toString();

            if ( ! formula.isEmpty() ) {
                if ( !refs.isEmpty() )
                    o << ", ";
                o << formula.toStdString();

                int idx = model_->data( model_->index( row, c_mspeaktable_index ) ).toInt();
                int fcn = model_->data( model_->index( row, c_mspeaktable_fcn ) ).toInt();

                refs.push_back( QPair<int,int>( idx, fcn ) );
            }
        }
        
        actions.push_back( menu.addAction( o.str().c_str() ) );
        QAction * selected = menu.exec( this->mapToGlobal( pt ) );
        if ( selected )
            emit triggerLockMass( refs );
        /*
        QString formula = model_->data( model_->index( index.row(), c_mspeaktable_formula ) ).toString();
        if ( !formula.isEmpty() ) {
            actions.push_back( menu.addAction( "Lock mass with this peak" ) );
            QAction * selected = menu.exec( this->mapToGlobal( pt ) );
            if ( selected ) {
                int idx = model_->data( model_->index( index.row(), c_mspeaktable_index ) ).toInt();
                int fcn = model_->data( model_->index( index.row(), c_mspeaktable_fcn ) ).toInt();
                emit triggerLockMass( idx, fcn );
            }
        } else {
            actions.push_back( menu.addAction( "Lock mass" ) );
            QAction * selected = menu.exec( this->mapToGlobal( pt ) );
            if ( selected ) {
                emit triggerLockMass( -1, -1 );
            }
        }
        */
    }
}

void
MSPeakTable::handleValueChanged( const QModelIndex& index )
{
    if ( inProgress_ )
        return;
    if ( index.column() == c_mspeaktable_formula ) {
        formulaChanged( index );
    }
    else if ( index.column() == c_mspeaktable_description ) {
        descriptionChanged( index );
    }
}

void
MSPeakTable::formulaChanged( const QModelIndex& index )
{
	QStandardItemModel& model = *model_;

    if ( index.column() == c_mspeaktable_formula ) {

        int fcn = model.index( index.row(), c_mspeaktable_fcn ).data( Qt::EditRole ).toInt();
        int idx = model.index( index.row(), c_mspeaktable_index ).data( Qt::EditRole ).toInt();

        std::string formula = index.data( Qt::EditRole ).toString().toStdString();
        double mass = model.index( index.row(), c_mspeaktable_mass ).data( Qt::EditRole ).toDouble();
        model.setData( model.index( index.row(), c_mspeaktable_mass_error ), mass - exactMass( formula ) );

        if ( data_source_.which() == 0 ) {
            auto wptr = boost::get< std::weak_ptr< adcontrols::MSPeakInfo > >( data_source_ );
            if ( auto ptr = wptr.lock() ) {
                adcontrols::segment_wrapper< adcontrols::MSPeakInfo > segs( *ptr );
                auto it = segs[ fcn ].begin() + idx;
                it->formula( formula );            
            }
        } else {
            auto wptr = boost::get< std::weak_ptr< adcontrols::MassSpectrum > >( data_source_ );
            if ( auto ptr = wptr.lock() ) {
                adcontrols::segment_wrapper<> segs( *ptr );
                if ( signed(segs.size()) > fcn ) {
                    auto& ms = segs[fcn];
                    adcontrols::annotations& annots = ms.get_annotations();
                    auto it = std::find_if( annots.begin(), annots.end(), [=]( const adcontrols::annotation& a ){
                            return a.index() == idx && a.dataFormat() == adcontrols::annotation::dataFormula; });
                    if ( it != annots.end() ) {
                        if ( formula.empty() )
                            annots.erase( it );
                        else
                            it->text( formula, adcontrols::annotation::dataFormula );
                    } else {
                        if ( !formula.empty() )
                            annots << adcontrols::annotation( formula
                                                            , ms.getMass( idx )
                                                            , ms.getIntensity( idx )
                                                            , idx
                                                            , 0
                                                            , adcontrols::annotation::dataFormula );
                    }
                    emit formulaChanged( idx, fcn );
                }
            }
        }
    }
}

void
MSPeakTable::descriptionChanged( const QModelIndex& index )
{
	QStandardItemModel& model = *model_;

    if ( index.column() == c_mspeaktable_description ) {

        int fcn = model.index( index.row(), c_mspeaktable_fcn ).data( Qt::EditRole ).toInt();
        int idx = model.index( index.row(), c_mspeaktable_index ).data( Qt::EditRole ).toInt();

        std::wstring description = index.data( Qt::EditRole ).toString().toStdWString();

        if ( data_source_.which() == 0 ) {
            auto wptr = boost::get< std::weak_ptr< adcontrols::MSPeakInfo > >( data_source_ );
            if ( auto ptr = wptr.lock() ) {
                adcontrols::segment_wrapper< adcontrols::MSPeakInfo > segs( *ptr );
                auto it = segs[ fcn ].begin() + idx;
                it->annotation( description );
            }
        } else {
            auto wptr = boost::get< std::weak_ptr< adcontrols::MassSpectrum > >( data_source_ );
            if ( auto ptr = wptr.lock() ) {
                adcontrols::segment_wrapper<> segs( *ptr );
                if ( signed(segs.size()) > fcn ) {
                    auto& ms = segs[fcn];
                    adcontrols::annotations& annots = ms.get_annotations();
                    auto it = std::find_if( annots.begin(), annots.end(), [=]( const adcontrols::annotation& a ){
                            return a.index() == idx && a.dataFormat() == adcontrols::annotation::dataText; });
                    if ( it != annots.end() ) {
                        if ( description.empty() )
                            annots.erase( it );
                        else
                            it->text( description, adcontrols::annotation::dataText );
                    } else {
                        if ( ! description.empty() )
                            annots << adcontrols::annotation( description
                                                          , ms.getMass( idx )
                                                          , ms.getIntensity( idx )
                                                          , idx
                                                          , 0
                                                          , adcontrols::annotation::dataText );
                    }
                    emit formulaChanged( idx, fcn );
                }
            }
        }
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

