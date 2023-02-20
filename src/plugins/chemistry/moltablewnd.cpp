/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "moltablewnd.hpp"
#include "chemquery.hpp"
#include "document.hpp"
#include <adchem/sdfile.hpp>
#include <adlog/logger.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/moltable.hpp>
#include <adprot/aminoacid.hpp>
#include <adwidgets/moltableview.hpp>

#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/MolSupplier.h>

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
#include <QSortFilterProxyModel>
#include <QSqlField>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QTextDocument>
#include <QUrl>
#include <QVBoxLayout>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/exception/all.hpp>

using namespace chemistry;

MolTableWnd::~MolTableWnd()
{
    delete model_;
}

MolTableWnd::MolTableWnd(QWidget *parent) : QWidget(parent)
                                          , model_( new QSqlQueryModel )
                                          , table_( new adwidgets::MolTableView )
{
    if ( auto layout = new QVBoxLayout( this ) ) {
        layout->setSpacing( 0 );
        layout->setContentsMargins( {} );
        layout->addWidget( table_ );
    }

    setAcceptDrops( true );

    if ( auto m = new QSortFilterProxyModel() ) {
        m->setDynamicSortFilter( true );
        m->setSourceModel( model_ );
        table_->setModel( m );
        table_->setSortingEnabled( true );
    }

    for ( auto& cname : { "id", "uuid" } )
        hideColumns_.insert( cname );

    table_->verticalHeader()->setDefaultSectionSize( 80 );
    table_->horizontalHeader()->setDefaultSectionSize( 200 );

    table_->setContextMenuHandler( [this]( const QPoint& pt ){ handleContextMenu( pt ); } );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QWidget::customContextMenuRequested, this, &MolTableWnd::handleContextMenu );

    connect( table_, &QTableView::activated, [&]( const QModelIndex& current ){
            emit activated( current );
        });

    // connect( document::instance(), &document::updateQuery, this, [&](){ model_->select(); } );
}

QAbstractItemModel *
MolTableWnd::model()
{
    return model_;
}

void
MolTableWnd::setQuery( const QString& sqlstmt )
{
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
            else if ( column == "synonym" )
                table_->setColumnField( col, adwidgets::ColumnState::f_synonym, false, true );
            else
                table_->setColumnField( col, adwidgets::ColumnState::f_any, false, false );
        }

        model_->setQuery( std::move( query ) );

        for ( auto& hidden: hideColumns_ ) {
            int col;
            if ( ( col = rec.indexOf( hidden ) ) >= 0 )
                table_->setColumnHidden( col, true );
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
    auto rec = model_->query().record();
    auto vCSID = model_->data( model_->index( index.row(), rec.indexOf( "csid" ) ) );

    if ( ! vCSID.isNull() ) {
        QString url = QString( tr( "http://www.chemspider.com/Chemical-Structure.%1.html" ) ).arg( vCSID.toInt() );
        actions.emplace_back( menu.addAction( url ), [=](){ QDesktopServices::openUrl( QUrl( url ) ); } );
    } else {
        auto vInChI = model_->data( model_->index( index.row(), rec.indexOf( "InChI" ) ) );
        if ( !vInChI.isNull() )
            actions.emplace_back( menu.addAction( tr("Get CSID") ), [=](){ document::instance()->findCSIDFromInChI( vInChI.toString() ); } );
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
    auto rec = model_->record(row);
    auto field = rec.field( column );
    return field.value();
}

void
MolTableWnd::handleDataChaged( const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector< int >& roles )
{

}
