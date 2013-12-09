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
#include <qtwrapper/waitcursor.hpp>

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
#include <QProgressBar>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/exception/all.hpp>

namespace chemistry { 
    static char * tags [] = {
        "DSSTox_RID"
        , "DSSTox_CID"
        , "DSSTox_Generic_SID"
        , "STRUCTURE_Formula"
        , "STRUCTURE_ChemicalName_IUPAC"
        , "STRUCTURE_MolecularWeight"
        , "STRUCTURE_ChemicalType"
        , "STRUCTURE_Shown"
        , "TestSubstance_ChemicalName"
        , "TestSubstance_CASRN" 
        , "TestSubstance_Description" 
        , "ChemicalNote" 
        , "STRUCTURE_InChIS" 
        , "STRUCTURE_InChIKey" 
        , "Substance_modify_yyyymmdd" 
    };
}

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
    model_->setColumnCount( 2 + sizeof(tags)/sizeof(tags[0]));

    verticalHeader()->setDefaultSectionSize( 80 );
    horizontalHeader()->setDefaultSectionSize( 200 );

    int col = 0;
    model_->setHeaderData( col++, Qt::Horizontal, "SMILES" );
    model_->setHeaderData( col++, Qt::Horizontal, "Structure" );
    for ( auto tag: tags )
        model_->setHeaderData( col++, Qt::Horizontal, tag );        

    for ( int c = 2; c < model_->columnCount(); ++c )
        resizeColumnToContents( c );
}

void
MolTableView::setMol( SDFile& file, QProgressBar& progressBar )
{
    if ( file ) {

        qtwrapper::waitCursor wait;

        RDKit::SDMolSupplier& supplier = file.molSupplier();
        model_->setRowCount( supplier.length() );

        progressBar.setRange( 0, supplier.length() );
        progressBar.setVisible( true );
        progressBar.setTextVisible( true );

        for ( size_t idx = 0; idx < supplier.length(); ++idx ) {
            progressBar.setValue( idx + 1 );
            try {
                if ( RDKit::ROMol * mol = supplier.next() ) {
                    std::string smiles = RDKit::MolToSmiles( *mol );
					if ( ! smiles.empty() ) {
                        model_->setData( model_->index( idx, 0 ), smiles.c_str() );
                        
                        // SVG
                        std::vector<int> drawing = RDKit::Drawing::MolToDrawing( *mol );
                        std::string svg = RDKit::Drawing::DrawingToSVG( drawing );
                        model_->setData( model_->index( idx, 1 ), QByteArray( svg.data(), svg.size() ) );
                    }
                    
                    // associated data
                    std::map< std::string, std::string > data;
                    SDFile::associatedData( supplier.getItemText( idx ), data );
                    
                    size_t col = 2;
                    for ( auto tag: tags ) {
                        auto it = data.find( tag );
                        if ( it != data.end() )
                            model_->setData( model_->index( idx, col ), it->second.c_str() );
                        ++col;
                    }
					delete mol;
                }
				if ( idx == 10 )
					this->update();
            } catch ( std::exception& ex ) {
                ADDEBUG() << boost::current_exception_diagnostic_information() << ex.what();
           } catch ( ... ) {
                ADDEBUG() << boost::current_exception_diagnostic_information();
            }
        }
        progressBar.setVisible( false );
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

