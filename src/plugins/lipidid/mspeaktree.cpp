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

#include "candidate.hpp"
#include "document.hpp"
#include "mol.hpp"
#include "mspeaktree.hpp"
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
#include <QDebug>
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

        void
        setCandidateData( QStandardItemModel * model, size_t row, const lipidid::candidate& candidate, QModelIndex parent ) {
            const auto& inchikey  = candidate.inchiKeys()[ 0 ];
            model->setData( model->index( row, c_formula, parent ), QString::fromStdString( candidate.formula() + candidate.adduct() ) );
            model->setData( model->index( row, c_exact_mass, parent ), candidate.exact_mass() );
            model->setData( model->index( row, c_mass_error, parent ), candidate.mass_error() * 1000 );
            model->setData( model->index( row, c_inchikey, parent ), QString::fromStdString( inchikey ) );
            model->setData( model->index( row, c_logP, parent ), moldb::logP( inchikey ) );
            if ( candidate.inchiKeys().size() > 1 ) {
                auto p = model->itemFromIndex( model->index( row, 0, parent ) );
                p->setColumnCount( 6 );
                p->setRowCount( candidate.inchiKeys().size() - 1 );
                for ( size_t j = 1; j < candidate.inchiKeys().size(); ++j ) {
                    const auto& inchikey = candidate.inchiKeys()[j];
                    model->setData( model->index( j - 1, c_inchikey, p->index() ), QString::fromStdString( inchikey ) );
                    model->setData( model->index( j - 1, c_logP, p->index() ), moldb::logP( inchikey ) );
                }
            }
        }

        std::optional<int> findFirstRow( const QStandardItemModel& model, int column, std::function< bool( const QVariant& ) > pred ) {
            for ( int i = 0; i < model.rowCount(); ++i ) {
                if ( pred( model.index( i, column ).data( Qt::EditRole ) ) )
                    return i;
            }
            return {};
        }

        std::optional<int> findLastRow( const QStandardItemModel& model, int column, std::function< bool( const QVariant& ) > pred ) {
            for ( int i = model.rowCount() -1; i >= 0; --i ) {
                if ( pred( model.index( i, column ).data( Qt::EditRole ) ) )
                    return i;
            }
            return {};
        }
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
    this->setSortingEnabled( false );
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
    setColumnHidden( c_index, true );
}


void
MSPeakTree::currentChanged( const QModelIndex& index, const QModelIndex& prev )
{
    scrollTo( index, QAbstractItemView::EnsureVisible );
    emit currentChanged( index );

    auto top = index;
    while ( top.parent() != QModelIndex() )
        top = top.parent();

    if ( index.column() == c_inchikey ) {
        auto key = index.data( Qt::EditRole ).toString();
        if ( ! key.isEmpty() ) {
            emit inChIKeySelected( key );
            auto fidx = index.model()->index( index.row(), c_formula, index.parent() );
            if ( fidx.data( Qt::EditRole ).isNull() )
                fidx = index.model()->index( fidx.parent().row(), c_formula, fidx.parent().parent() );
            if ( fidx.data( Qt::EditRole ).isValid() ) {
                auto formula = fidx.data( Qt::EditRole ).toString();
                double abundance = index.model()->index( top.row(), c_intensity ).data( Qt::EditRole ).toDouble();
                document::instance()->handleFormulaSelected( formula, abundance );
            }
        }
    }
    if ( index.column() == c_formula ) {
        auto formula = index.data( Qt::EditRole ).toString();
        if ( ! formula.isEmpty() ) {
            double abundance = index.model()->index( top.row(), c_intensity ).data( Qt::EditRole ).toDouble();
            document::instance()->handleFormulaSelected( formula, abundance );

            auto key = index.model()->index( index.row(), c_inchikey ).data( Qt::EditRole ).toString();
            if ( !key.isEmpty() ) {
                emit inChIKeySelected( key );
            }
        }

    }
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
MSPeakTree::handleZoomedOnSpectrum( int view, const QRectF& rc )
{
    QStandardItemModel& model = *impl_->model_;
    // std::shared_ptr< const lipidid::simple_mass_spectrum > simple_mass_spectrum;
    // std::tie( std::ignore, std::ignore, simple_mass_spectrum ) = document::instance()->getResultSet();
    // if ( simple_mass_spectrum ) {
    //     auto it = std::lower_bound( simple_mass_spectrum->begin()
    //                                 , simple_mass_spectrum->end()
    //                                 , rc.left()
    //                                 , [](const auto& a, double left){ return mass_value_t::mass(a) < left; });
    // }
    auto visualRows = std::make_pair( indexAt( rect().topLeft() ).row(), indexAt( rect().bottomLeft() ).row() );
    if ( rc.left() < model.index( c_mass, visualRows.first ).data().toDouble() && // view left is lower than tree-top
         rc.right() > model.index( c_mass, visualRows.second ).data().toDouble() ) { // view right is higher than tree-bottom
        // tree range is contained -- nothing to do
    }
    if ( rc.left() > model.index( c_mass, visualRows.first ).data().toDouble() ) { // view left is higher than tree-top
        if ( auto row = findFirstRow( model, c_mass, [&](auto& v){ return v.toDouble() > rc.left(); }) ) {
            scrollTo( model.index( *row, 0 ), EnsureVisible );
        }
    } else if ( rc.right() < model.index( c_mass, visualRows.second ).data().toDouble() ) { // view right is lower than tree-bottom
        if ( auto row = findLastRow( model, c_mass, [&](auto& v){ return v.toDouble() < rc.right(); }) ) {
            scrollTo( model.index( *row, 0 ), EnsureVisible );
        }
    }
    if ( view == 1 ) {
        // find base peak and select it
        auto leftIndex = findFirstRow( model, c_mass, [&](auto& v){ return v.toDouble() > rc.left(); });
        auto rightIndex = findLastRow( model, c_mass, [&](auto& v){ return v.toDouble() < rc.right(); });
        if ( leftIndex && rightIndex ) {
            int sel( *leftIndex );
            for ( int row = *leftIndex; row <= *rightIndex; ++row ) {
                if ( model.index( row, c_intensity ).data().toDouble() > model.index( sel, c_intensity ).data().toDouble() ) {
                    sel = row;
                }
            }
            // ADDEBUG() << "selected row: " << sel << ", mass=" << model.index( sel, c_mass ).data().toDouble();
            scrollTo( model.index( sel, 0 ), EnsureVisible );
            selectionModel()->select( model.index( sel, 0 ), QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}

void
MSPeakTree::handleCopyAllToClipboard()
{
    QStandardItemModel& model = *impl_->model_;

    QString selected_text;
    for ( int col = 0; col < model.columnCount(); ++col ) {
        selected_text.append( model.headerData( col, Qt::Horizontal ).toString() );
        selected_text.append( '\t' );
    }
    selected_text.append( '\n' );

    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( ! isRowHidden( row, QModelIndex() ) ) {
            for ( int col = 0; col < model.columnCount(); ++col ) {
                selected_text.append( model.index( row, col ).data( Qt::EditRole ).toString() );
                if ( col != model.columnCount() - 1 )
                    selected_text.append( '\t' );
            }
            selected_text.append( '\n' );

            if ( auto parent = model.itemFromIndex( model.index( row, 0 ) ) ) {
                for ( auto i = 0; i < parent->rowCount(); ++i ) {
                    for ( int col = 0; col < model.columnCount(); ++col ) {
                        selected_text.append( model.index( i, col, parent->index() ).data( Qt::EditRole ).toString() );
                        if ( col != model.columnCount() - 1 )
                            selected_text.append( '\t' );
                    }
                    selected_text.append( '\n' );
                }
            }
        }
    }
    QApplication::clipboard()->setText( selected_text );
}

void
MSPeakTree::handleCopyCheckedToClipboard()
{
    QStandardItemModel& model = *impl_->model_;

    QString selected_text;

    for ( int col = 0; col < model.columnCount(); ++col ) {
        selected_text.append( model.headerData( col, Qt::Horizontal ).toString() );
        selected_text.append( '\t' );
    }
    selected_text.append( '\n' );

    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( model.item( row, 0 )->checkState() == Qt::Checked ) {
            for ( int col = 0; col < model.columnCount(); ++col ) {
                selected_text.append( model.index( row, col ).data( Qt::EditRole ).toString() );
                if ( col != model.columnCount() - 1 )
                    selected_text.append( '\t' );
            }
            selected_text.append( '\n' );

            if ( auto parent = model.item( row, 0 ) ) {
                for ( auto i = 0; i < parent->rowCount(); ++i ) {
                    for ( int col = 0; col < model.columnCount(); ++col ) {
                        selected_text.append( model.index( i, col, parent->index() ).data( Qt::EditRole ).toString() );
                        if ( col != model.columnCount() - 1 )
                            selected_text.append( '\t' );
                    }
                    selected_text.append( '\n' );
                }
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
        menu.addAction( tr("Copy Checked"), this, SLOT( handleCopyCheckedToClipboard() ) );
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

    size_t row(0);
    for ( size_t i = 0; i < simple_mass_spectrum->size(); ++i ){
        auto [ time, mass, abundance, color ] = (*simple_mass_spectrum)[ i ];
        (void)time;
        auto candidates = simple_mass_spectrum->candidates( i );
        if ( ! candidates.empty() ) {
            model->setData( model->index( row, c_index ), int( i ) );
            model->setData( model->index( row, c_mass ), mass );
            model->setData( model->index( row, c_intensity ), abundance );
            if ( auto item = model->item( row, 0 ) ) {
                item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                item->setData( Qt::Unchecked, Qt::CheckStateRole );
            }
            // setRowHidden( i, QModelIndex(), candidates.empty() );
            if ( candidates.size() == 1 ) {
                setCandidateData( model, row, candidates.at( 0 ), QModelIndex() );
            } else {
                auto parent = model->itemFromIndex( model->index( row, 0 ) );
                parent->setColumnCount( c_num_columns );
                parent->setRowCount( candidates.size() );
                size_t k(0);
                for ( const auto& candidate: candidates ) { // for ( size_t k = 0; k < candidates.size(); ++k ) {
                    setCandidateData( model, k, candidate, parent->index() );
                    ++k;
                }
            }
            ++row;
        }
    }
    resizeColumnToContents( c_formula );
    resizeColumnToContents( c_inchikey );
}

void
MSPeakTree::handleDataChanged( const portfolio::Folium& folium )
{
}

#include "mspeaktree.moc"
