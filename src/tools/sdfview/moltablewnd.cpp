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
#include "document.hpp"
#include "moltabledelegate.hpp"
#include "moltablewnd.hpp"
#include <adchem/sdfile.hpp>
#include <adlog/logger.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/moltable.hpp>
#include <adprot/aminoacid.hpp>
#include <adportable/debug.hpp>
#include <adwidgets/moltableview.hpp>
#include <adwidgets/progresswnd.hpp>

#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/inchi.h>

#include <adchem/drawing.hpp>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QMimeData>
#include <QProgressBar>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QSqlField>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QStandardItemModel>
#include <QTextDocument>
#include <QUrl>
#include <QVBoxLayout>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/exception/all.hpp>
#include <future>
#include <thread>

using namespace sdfview;

MolTableWnd::~MolTableWnd()
{
}

MolTableWnd::MolTableWnd(QWidget *parent) : QWidget(parent)
                                          , model_( std::make_unique< QSqlQueryModel >() )
                                          , table_( new adwidgets::MolTableView )
                                          , backup_( 0 )
{
    backup_ = table_->itemDelegate();

    if ( auto layout = new QVBoxLayout( this ) ) {
        layout->setSpacing( 0 );
        layout->setMargin( 0 );
        layout->addWidget( table_ );
    }

    setAcceptDrops( true );

    QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::Copy), table_);
    connect( shortcut, SIGNAL(activated()), table_, SLOT( handleCopyToClipboard() ) );

    // table_->setModel( model_ );
    if ( auto m = new QSortFilterProxyModel() ) {
        m->setDynamicSortFilter( true );
        m->setSourceModel( model_.get() );
        table_->setModel( m );
        table_->setSortingEnabled( true );
    }

    table_->verticalHeader()->setDefaultSectionSize( 200 );
    table_->horizontalHeader()->setDefaultSectionSize( 200 );

    table_->setContextMenuHandler( [this]( const QPoint& pt ){ handleContextMenu( pt ); } );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QWidget::customContextMenuRequested, this, &MolTableWnd::handleContextMenu );

    connect( table_, &QTableView::activated, [&]( const QModelIndex& current ){
            emit activated( current );
        });
}

QAbstractItemModel *
MolTableWnd::model()
{
    return model_.get();
}

void
MolTableWnd::setQuery( const QString& sqlstmt )
{
    ADTRACE() << "setQuery: " << sqlstmt.toStdString();

    if ( auto model = qobject_cast< QSqlQueryModel * >( model_.get() ) ) {

        QSqlQuery query( sqlstmt, document::instance()->sqlDatabase() );

        if ( query.exec() ) {
            auto rec = query.record();

            for ( int col = 0; col < rec.count(); ++col ) {

                auto column = rec.fieldName(col);

                if ( column == "svg" )
                    table_->setColumnField( col, adwidgets::ColumnState::f_svg, false, false );
                else if ( column == "mass" )
                    table_->setColumnField( col, adwidgets::ColumnState::f_mass, false, false );
                else if ( column == "formula" )
                    table_->setColumnField( col, adwidgets::ColumnState::f_formula, false, true );
                else if ( column == "smiles" )
                    table_->setColumnField( col, adwidgets::ColumnState::f_smiles, false, true );
                else
                    table_->setColumnField( col, adwidgets::ColumnState::f_any, false, false );
            }

            model->setQuery( query );
        }
    }

}

void
MolTableWnd::dragEnterEvent( QDragEnterEvent * event )
{
	const QMimeData * mimeData = event->mimeData();

	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
        for ( auto& url: urlList ) {
            boost::filesystem::path path( url.toLocalFile().toStdWString() );
            if ( path.extension() == L".sdf" || path.extension() == L".mol" ) {
                event->accept();
                return;
            }
        }
	}
}

void
MolTableWnd::dragMoveEvent( QDragMoveEvent * event )
{
    event->accept();
}

void
MolTableWnd::dragLeaveEvent( QDragLeaveEvent * event )
{
	event->accept();
}

void
MolTableWnd::dropEvent( QDropEvent * event )
{
	const QMimeData * mimeData = event->mimeData();

	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
        emit dropped( urlList );
        event->accept();
	}
}

void
MolTableWnd::handlePaste()
{
}

void
MolTableWnd::handleContextMenu( const QPoint& pt )
{
    QMenu menu;

    auto action = menu.addAction( tr("Copy"), table_, SLOT( handleCopyToClipboard() ) );
    // menu.addAction( tr("Paste"), this, SLOT( handlePaste() ) );
    if ( table_->selectionModel()->selectedIndexes().size() == 0 )
        action->setEnabled( false );

    typedef std::pair< QAction *, std::function< void() > > action_type;
    std::vector< action_type > actions;

    QModelIndex index = table_->currentIndex();
    if ( auto model = qobject_cast< QSqlQueryModel *>( model_.get() ) ) {
        auto rec = model->query().record();
        auto vCSID = model->data( model_->index( index.row(), rec.indexOf( "csid" ) ) );
        if ( ! vCSID.isNull() ) {
            QString url = QString( tr( "http://www.chemspider.com/Chemical-Structure.%1.html" ) ).arg( vCSID.toInt() );
            actions.emplace_back( menu.addAction( url ), [=](){ QDesktopServices::openUrl( QUrl( url ) ); } );
        } else {
            auto vInChI = model_->data( model_->index( index.row(), rec.indexOf( "InChI" ) ) );
        }
    }

    if ( QAction * selected = menu.exec( mapToGlobal( pt ) ) ) {
        auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){
                return t.first == selected;
            });
        if ( it != actions.end() )
            (it->second)();
    }
}

QVariant
MolTableWnd::data( int row, const QString& column )
{
    if ( auto model = qobject_cast< QSqlQueryModel *>( model_.get() ) ) {
        auto rec = model->record(row);
        auto field = rec.field( column );
        return field.value();
    }
    return {};
}

void
MolTableWnd::handleDataChaged( const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector< int >& roles )
{
}

void
MolTableWnd::handleSDFileChanged()
{
    auto tp = std::chrono::steady_clock::now();
    emit onProgressInitiated( document::instance()->sdfile()->size() );
    std::atomic< uint32_t > progressCount(0);
    auto future = std::async( std::launch::async, [&](){
        return document::instance()->sdfile()->populate( [&](size_t c){ progressCount = c; } );
    });
    using namespace std::chrono_literals;
    while ( std::future_status::ready != future.wait_for( 200ms ) ) {
        emit onProgress( progressCount );
        QCoreApplication::instance()->processEvents();
    }
    future.wait();

    ADDEBUG() << "elapsed time: "
              << double( std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - tp ).count() ) / 1000.0
              << " s";
    emit onProgressFinished();
    QCoreApplication::instance()->processEvents();

    document::instance()->set_sdmols( future.get() );
    auto sdmols = document::instance()->sdmols();
    auto model = std::make_unique< QStandardItemModel >();
    std::vector< std::string > keys{ "svg", "formula", "mass", "smiles" };
    for ( auto it = sdmols.begin(); it != sdmols.end(); ++it ) {
        for ( const auto& item: it->dataItems() ) {
            if ( std::find( keys.begin(), keys.end(), item.first ) == keys.end() )
                keys.emplace_back( item.first );
        }
    }

    auto delegate = new MolTableDelegate();
    connect( delegate, &MolTableDelegate::onNullData, this, &MolTableWnd::handleNullData );

    delegate->setColumnField( 0, adwidgets::ColumnState::f_svg, false, false );
    delegate->setColumnField( 1, adwidgets::ColumnState::f_formula, false, true );
    delegate->setColumnField( 2, adwidgets::ColumnState::f_mass, false, false );
    delegate->setColumnField( 3, adwidgets::ColumnState::f_smiles, false, true );

    table_->setItemDelegate( delegate );

    model->setColumnCount( keys.size() );
    model->setRowCount( sdmols.size() );
    for ( auto it = keys.begin(); it != keys.end(); ++it ) {
        model->setHeaderData( std::distance( keys.begin(), it ), Qt::Horizontal, QString::fromStdString( *it ) );
    }

    emit onProgressInitiated( document::instance()->sdfile()->size() );
    size_t row(0);
    for ( auto it = sdmols.begin(); it != sdmols.end(); ++it, ++row ) {
        emit onProgress( row );
        // following data will be loaded on demand
        //model->setData( model->index( row, 0 ), QByteArray( it->svg().data(), it->svg().size() ) );
        model->setData( model->index( row, 1 ), QString::fromStdString( it->formula() ) );
        model->setData( model->index( row, 2 ), adcontrols::ChemicalFormula().getMonoIsotopicMass( it->formula() ) );
        //model->setData( model->index( row, 3 ), QString::fromStdString( it->smiles() ) );
        for ( const auto& item: it->dataItems() ) {
            auto col = std::distance( keys.begin(), std::find( keys.begin(), keys.end(), item.first ) );
            model->setData( model->index( row, col ), QString::fromStdString( item.second ) );
        }
    }
    emit onProgressFinished();

    if ( auto m = new QSortFilterProxyModel() ) {
        m->setDynamicSortFilter( true );
        m->setSourceModel( model.get() );
        table_->setModel( m );
        table_->setSortingEnabled( true );
    }
    // table_->setModel( model.get() );
    // table_->setSortingEnabled( true );
    model_ = std::move( model ); // replace model_ should be later than table model replace
    table_->verticalHeader()->setFixedWidth( 40 );
    table_->verticalHeader()->setSizeAdjustPolicy( QHeaderView::AdjustToContents );

    ADDEBUG() << "total elapsed time: "
              << double( std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - tp ).count() ) / 1000.0
              << " s";
}

void
MolTableWnd::handleNullData( const QModelIndex& index )
{
    // demand data loadgin
    auto sdmol = document::instance()->sdmols().at( index.row() );

    table_->model()->setData( table_->model()->index( index.row(), 0 ), QByteArray( sdmol.svg().data(), sdmol.svg().size() ) );
    // table_->model()->setData( table_->model()->index( index.row(), 1 ), QString::fromStdString( sdmol.formula() ) );
    // table_->model()->setData( table_->model()->index( index.row(), 2 ), adcontrols::ChemicalFormula().getMonoIsotopicMass( sdmol.formula() ) );
    table_->model()->setData( table_->model()->index( index.row(), 3 ), QString::fromStdString( sdmol.smiles() ) );
}
