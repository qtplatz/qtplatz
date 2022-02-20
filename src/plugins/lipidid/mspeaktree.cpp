/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "document.hpp"
#include "candidate.hpp"
#include "simple_mass_spectrum.hpp"
#include <adwidgets/htmlheaderview.hpp>
#include <adwidgets/delegatehelper.hpp>
#include <adwidgets/grid_render.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/annotation.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/targeting.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/is_type.hpp>
#include <adportfolio/folium.hpp>
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
#include <boost/signals2.hpp>
#include <boost/variant.hpp>
#include <boost/json.hpp>
#include <algorithm>
#include <sstream>
#include <set>
#include <ratio>

namespace lipidid {

	using namespace adcontrols::metric;

    namespace {

        enum {
            c_mass
            , c_intensity
            , c_formula
            , c_inchikey
            , c_logP
            , c_exact_mass
            , c_mass_error
            , c_index
            , c_num_columns
        };

        class delegate : public QItemDelegate {
            Q_OBJECT
        public:
            explicit delegate( QObject *parent = 0 ) : QItemDelegate( parent ) {
            }

            void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
                QStyleOptionViewItem op( option );
                op.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

                switch( index.column() ) {
                case c_exact_mass:
                case c_mass:
                    if ( index.data( Qt::EditRole ).toDouble() > 0.1 ) {
                        drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'g', 7 ) );
                    } else {
                        drawDisplay( painter, op, option.rect, "" );
                    }
                    break;
                case c_mass_error:
                    drawDisplay( painter, op, option.rect, QString::number( index.data( Qt::EditRole ).toDouble(), 'g', 4 ) );
                    break;
                case c_intensity:
                    if ( index.data( Qt::EditRole ).toDouble() > 0 ) {
                        drawDisplay( painter, op, option.rect, (boost::format( "%.2lf" ) % (index.data( Qt::EditRole ).toDouble())).str().c_str() );
                    } else {
                        drawDisplay( painter, op, option.rect, "" );
                    }
                    break;
                case c_formula:
                    do {
                        std::string formula = adcontrols::ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                        using adwidgets::DelegateHelper;
                        DelegateHelper::render_html( painter, option, QString::fromStdString( formula ) );
                    } while(0);
                    break;
                default:
                    QItemDelegate::paint( painter, option, index );
                    break;
                }
            }

            QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                auto size = QItemDelegate::sizeHint(option, index);
                if ( index.parent() == QModelIndex() )
                    size.setHeight(20);
                return size;
            }

        public slots:

        };


        template<class T> struct lock_weak_pointer : public boost::static_visitor< std::shared_ptr<T> > {
            std::shared_ptr< T > operator ()( std::weak_ptr<T>& wptr ) const {
                return wptr.lock();
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

namespace lipidid {

    class MSPeakTree::impl {
    public:
        std::unique_ptr< QStandardItemModel > model_;
        std::unique_ptr< QItemDelegate > delegate_;

        impl() : model_( std::make_unique< QStandardItemModel >() )
               , delegate_( std::make_unique< delegate >() ) {
        }

        bool inProgress_;
    };
}

using namespace lipidid;

MSPeakTree::~MSPeakTree()
{
    delete impl_;
}

MSPeakTree::MSPeakTree(QWidget *parent) : QTreeView( parent )
                                        , impl_( new impl() )
{
    using adwidgets::HtmlHeaderView;

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

void
MSPeakTree::OnFinalClose()
{
}

void
MSPeakTree::onInitialUpdate()
{
    QStandardItemModel& model = *impl_->model_;

    model.setColumnCount( c_num_columns );

    model.setHeaderData( c_index,       Qt::Horizontal, QObject::tr( "index" ) ); //time(&mu;s)" ) );
    model.setHeaderData( c_mass,        Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
    model.setHeaderData( c_intensity,   Qt::Horizontal, QObject::tr( "Abundance" ) );
    model.setHeaderData( c_formula,     Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_inchikey,    Qt::Horizontal, QObject::tr( "InChIkey" ) );
    model.setHeaderData( c_exact_mass,  Qt::Horizontal, QObject::tr( "Exact <i>m/z</i>" ) );
    model.setHeaderData( c_mass_error,  Qt::Horizontal, QObject::tr( "error(mDa)" ) );
    model.setHeaderData( c_logP,        Qt::Horizontal, QObject::tr( "logP" ) );
    // model.setHeaderData( c_delta_mass,  Qt::Horizontal, QObject::tr( "&delta;Da" ) );

    // model.setHeaderData( c_relative_intensity, Qt::Horizontal, QObject::tr( "R.A. (%)" ) );
    // model.setHeaderData( c_exact_abundance,    Qt::Horizontal, QObject::tr( "R.A. (exact,%)" ) );
    // model.setHeaderData( c_abundance_error,    Qt::Horizontal, QObject::tr( "R.A. Error (%)" ) );
    // model.setHeaderData( c_mode,        Qt::Horizontal, QObject::tr( "mode" ) );
    setColumnHidden( c_index, true );
}


void
MSPeakTree::currentChanged( const QModelIndex& index, const QModelIndex& prev )
{
    // QStandardItemModel& model = *impl_->model_;
    // (void)prev;
    scrollTo( index, QAbstractItemView::EnsureVisible );
    emit currentChanged( index.data( Qt::EditRole ).toString() );
    // int idx = model.index( row, c_index ).data( Qt::EditRole ).toInt();
    // emit currentChanged( idx );
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
#if 0
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
                    if ( model.index( row, c_index ).data( Qt::EditRole ).toInt() == bp.first ) {
                        QModelIndex index = model.index( row, isTimeAxis ? c_time : c_mass );
                        setCurrentIndex( index );
                        scrollTo( index, QAbstractItemView::EnsureVisible );
                        break;
                    }
                }
            } while ( 0 );
        }
    }
#endif
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
		if ( model.data( idx ).type() == QVariant::Double )
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
    QModelIndex index = currentIndex();

	if ( index.isValid() ) {

        QMenu menu;

        QModelIndexList list = selectionModel()->selectedIndexes();
        if ( list.size() < 1 )
            return;

        //--------------------
        std::set< int > rows;
        for ( auto index: list )
            rows.insert( index.row() ); // make unique row list

        //------------ Copy assigned
        menu.addAction( tr("Copy All"), this, SLOT( handleCopyAllToClipboard() ) );
        menu.addAction( tr("Copy Selected"), this, SLOT( handleCopyToClipboard() ) );

        menu.exec( this->mapToGlobal( pt ) );
    }
}


void
MSPeakTree::addContextMenu(QMenu &, const QPoint &, std::shared_ptr<const adcontrols::MassSpectrum>) const
{
}

void
MSPeakTree::handleIdCompleted()
{
    auto [ ms, refms, simple_mass_spectrum ] = document::instance()->getResultSet();

    auto obj = boost::json::object{{ "simple_mass_spectrum", *simple_mass_spectrum }};

    auto model = impl_->model_.get();
    model->setRowCount( simple_mass_spectrum->size() );

    for ( size_t i = 0; i < simple_mass_spectrum->size(); ++i ){
        auto [ time, mass, abundance, color ] = (*simple_mass_spectrum)[ i ];
        (void)time;

        model->setData( model->index( i, c_index ), int( i ) );
        model->setData( model->index( i, c_mass ), mass );
        model->setData( model->index( i, c_intensity ), abundance );

        auto candidates = simple_mass_spectrum->candidates( i );
        setRowHidden( i, QModelIndex(), candidates.empty() );
        if ( ! candidates.empty() ) {
            if ( candidates.size() == 1 ) {
                const auto& candidate = candidates.at( 0 );
                const auto& inchikey  = candidate.inchiKeys[ 0 ];
                //const auto& mols = candidate.mols();
                model->setData( model->index( i, c_formula ), QString::fromStdString( candidate.formula + candidate.adduct ) );
                model->setData( model->index( i, c_exact_mass ), candidate.exact_mass );
                model->setData( model->index( i, c_mass_error ), candidate.mass_error * 1000 );
                model->setData( model->index( i, c_inchikey ), QString::fromStdString( inchikey ) );
                model->setData( model->index( i, c_logP ), document::instance()->logP( inchikey ) );
                if ( candidate.inchiKeys.size() > 1 ) {
                    auto p = model->itemFromIndex( model->index( i, 0 ) );
                    p->setColumnCount( 6 );
                    p->setRowCount( candidate.inchiKeys.size() - 1 );
                    for ( size_t j = 1; j < candidate.inchiKeys.size(); ++j ) {
                        const auto& inchikey = candidate.inchiKeys[j];
                        model->setData( model->index( j - 1, c_inchikey, p->index() ), QString::fromStdString( inchikey ) );
                        model->setData( model->index( j - 1, c_logP, p->index() ), document::instance()->logP( inchikey ) );
                    }
                }
            } else {
                auto parent = model->itemFromIndex( model->index( i, 0 ) );
                parent->setColumnCount( c_num_columns );
                parent->setRowCount( candidates.size() );
                size_t k(0);
                for ( const auto& candidate: candidates ) { // for ( size_t k = 0; k < candidates.size(); ++k ) {
                    const auto& inchikey = candidate.inchiKeys[0];
                    ADDEBUG() << boost::json::object{{ "x", candidate }};
                    model->setData( model->index( k, c_formula, parent->index() ), QString::fromStdString( candidate.formula ) );
                    model->setData( model->index( k, c_exact_mass, parent->index() ), candidate.exact_mass );
                    model->setData( model->index( k, c_mass_error, parent->index() ), candidate.mass_error * 1000 );
                    model->setData( model->index( k, c_inchikey, parent->index() ), QString::fromStdString( inchikey ) );
                    model->setData( model->index( k, c_logP ), document::instance()->logP( inchikey ) );
                    if ( candidate.inchiKeys.size() > 1 ) {
                        auto p = model->itemFromIndex( model->index( k, 0, parent->index() ) );
                        p->setColumnCount( c_num_columns );
                        p->setRowCount( candidate.inchiKeys.size() - 1 );
                        for ( size_t j = 1; j < candidate.inchiKeys.size(); ++j ) {
                            model->setData( model->index( j - 1, c_inchikey, p->index() ), QString::fromStdString( candidate.inchiKeys[j] ) );
                            model->setData( model->index( j - 1, c_logP, p->index() ), document::instance()->logP( candidate.inchiKeys[j] ) );
                        }
                    }
                    ++k;
                }
            }
        }
    }
    ADDEBUG() << "tree updated";
    resizeColumnToContents( c_formula );
    resizeColumnToContents( c_inchikey );
}

void
MSPeakTree::handleDataChanged( const portfolio::Folium& folium )
{
   using portfolio::is_any_shared_of;
    if ( is_any_shared_of< adcontrols::MassSpectrum, const adcontrols::MassSpectrum >( folium ) ) {
        using portfolio::get_shared_of;
        if ( auto ptr = get_shared_of< const adcontrols::MassSpectrum, adcontrols::MassSpectrum >()( folium.data() ) ) {
            if ( ptr->isCentroid() ) {
                auto model = impl_->model_.get();
                model->setRowCount( ptr->size() );
                for ( size_t i = 0; i < ptr->size(); ++i ){
                    auto [ time, mass, abundance, color ] = ptr->value( i );
                    (void)time;

                    model->setData( model->index( i, c_index ), int( i ) );
                    model->setData( model->index( i, c_mass ), mass );
                    model->setData( model->index( i, c_intensity ), abundance );
                    setRowHidden( i, QModelIndex(), color != 15 );
                }
            }
        }
    }
}



#include "mspeaktree.moc"
