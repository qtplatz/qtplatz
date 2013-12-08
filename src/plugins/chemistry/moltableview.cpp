/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "moltableview.hpp"
#include "moltabledelegate.hpp"
#include "sdfile.hpp"
#include <adportable/debug.hpp>

//#include <GraphMol/SmilesParse/SmilesParse.h>
#include <RDGeneral/Invariant.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/FileParsers/FileParsers.h>
//#include <RDGeneral/RDLog.h>

#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/MolDrawing/MolDrawing.h>
#include <GraphMol/MolDrawing/DrawingToSVG.h>

#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QDebug>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QByteArray>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace chemistry;

MolTableView::~MolTableView()
{
    delete model_;
    delete delegate_;
}

MolTableView::MolTableView(QWidget *parent) : QTableView(parent)
                                            , delegate_( new MolTableDelegate )
                                            , model_( new QStandardItemModel )
{
    setAcceptDrops( true );
    setModel( model_ );
    setItemDelegate( delegate_ );
    model_->setColumnCount( 2 );

    verticalHeader()->setDefaultSectionSize( 80 );
    horizontalHeader()->setDefaultSectionSize( 200 );
}

void
MolTableView::setMol( SDFile& file )
{
    if ( file ) {
        RDKit::SDMolSupplier& supplier = file.molSupplier();
        model_->setRowCount( supplier.length() );

        for ( size_t idx = 0; idx < supplier.length(); ++idx ) {

            if ( RDKit::ROMol * mol = supplier.next() ) {
                
                std::string smiles = RDKit::MolToSmiles( *mol );
                model_->setData( model_->index( idx, 0 ), smiles.c_str() );
                
                //std::string text = supplier.getItemText( idx );
                //ADP_DEBUG() << text;

                std::vector<int> drawing = RDKit::Drawing::MolToDrawing( *mol );
                std::string svg = RDKit::Drawing::DrawingToSVG( drawing );
                model_->setData( model_->index( idx, 1 ), QByteArray( svg.data(), svg.size() ) );

                std::map< std::string, std::string > data;
                SDFile::associatedData( supplier.getItemText( idx ), data );

                delete mol;

                if ( idx == 1 )
                    break;
            }
        }
    }
}

void
MolTableView::dragEnterEvent( QDragEnterEvent * event )
{
	const QMimeData * mimeData = event->mimeData();

	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
        for ( auto& url: urlList ) {
            boost::filesystem::path path( url.toLocalFile().toStdWString() );
            if ( path.extension() == L".sdf" ) {
                event->accept();
                return;
            }
        }
	}
}

void
MolTableView::dragMoveEvent( QDragMoveEvent * event )
{
    event->accept();
}

void
MolTableView::dragLeaveEvent( QDragLeaveEvent * event )
{
	event->accept();
}

void
MolTableView::dropEvent( QDropEvent * event )
{
	const QMimeData * mimeData = event->mimeData();

	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
        emit dropped( urlList );
        event->accept();
	}
}

