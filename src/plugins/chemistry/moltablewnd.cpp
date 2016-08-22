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

#include <compiler/diagnostic_push.h>
#if defined _MSC_VER
# pragma warning(disable:4267) // size_t to unsigned int possible loss of data (x64 int on MSC is 32bit)
#endif
#include <compiler/disable_deprecated.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <compiler/diagnostic_pop.h>

#include <adchem/drawing.hpp>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QMimeData>
#include <QProgressBar>
#include <QSqlField>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlRecord>
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
        layout->setMargin( 0 );
        layout->addWidget( table_ );
    }
    
    setAcceptDrops( true );

    table_->setModel( model_ );

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
            else
                table_->setColumnField( col, adwidgets::ColumnState::f_any, false, false );
        }
        
        model_->setQuery( query );

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
MolTableWnd::handleCopyToClipboard()
{
#if 0
	QModelIndexList indecies = table_->selectionModel()->selectedIndexes();

    qSort( indecies );
    if ( indecies.size() < 1 )
        return;

    adcontrols::moltable molecules;
    
    QString selected_text;
    QModelIndex prev = indecies.first();
    QModelIndex last = indecies.last();

    indecies.removeFirst();

    adcontrols::moltable::value_type mol;
    auto query = ChemDocument::instance()->query();

    for( int i = 0; i < indecies.size(); ++i ) {
        
        QModelIndex index = indecies.at( i );

        if ( !table_->isRowHidden( prev.row() ) ) {

            auto t = prev.data( Qt::EditRole ).type();
            if ( !table_->isColumnHidden( prev.column() ) && ( prev.data().type() != QVariant::ByteArray ) ) {

                QString text = prev.data( Qt::EditRole ).toString();
                selected_text.append( text );

                if ( index.row() == prev.row() )
                    selected_text.append( '\t' );
            }
            if ( query ) {
                int col = prev.column();
                auto cname = query->column_name( col );
                if ( cname == "formula" ) {
                    mol.formula() = prev.data( Qt::EditRole ).toString().toStdString();
                } else if ( cname == "synonym" ) {
                    mol.synonym() = prev.data( Qt::EditRole ).toString().toStdString();
                } else if ( cname == "smiles" ) {
                    mol.smiles() = prev.data( Qt::EditRole ).toString().toStdString();
                } else if ( cname == "mass" ) {
                    mol.mass() = prev.data( Qt::EditRole ).toDouble();
                }
            }
            
            if ( index.row() != prev.row() ) {
                selected_text.append( '\n' );
                molecules << mol;
                mol = adcontrols::moltable::value_type();
            }
        }
        prev = index;
    }

    if ( !table_->isRowHidden( last.row() ) && !table_->isColumnHidden( last.column() ) )
        selected_text.append( last.data( Qt::EditRole ).toString() );

    QApplication::clipboard()->setText( selected_text );
    
    std::wostringstream o;
    try {
        if ( adcontrols::moltable::xml_archive( o, molecules ) ) {
            QString xml( QString::fromStdWString( o.str() ) );
            QMimeData * md = new QMimeData();
            md->setData( QLatin1String( "application/moltable-xml" ), xml.toUtf8() );
            md->setText( selected_text );
            QApplication::clipboard()->setMimeData( md, QClipboard::Clipboard );
        }
    } catch ( ... ) {
    }
#endif
}

void
MolTableWnd::handlePaste()
{
}

void
MolTableWnd::handleContextMenu( const QPoint& pt )
{
    QMenu menu;

    menu.addAction( tr("Copy"), this, SLOT( handleCopyToClipboard() ) );
    menu.addAction( tr("Paste"), this, SLOT( handlePaste() ) );    
    
    typedef std::pair< QAction *, std::function< void() > > action_type;
    std::vector< action_type > actions;

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
