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

#include "moltableview.hpp"
#include "moltabledelegate.hpp"
#include <adchem/sdfile.hpp>
#include <adlog/logger.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adprot/aminoacid.hpp>

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
    static const char * tags [] = {
        "STRUCTURE_ChemicalName_IUPAC"
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
        , "DSSTox_RID"
        , "DSSTox_CID"
        , "DSSTox_Generic_SID"
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

	setSortingEnabled( true );

    model_->setColumnCount( 2 + sizeof(tags)/sizeof(tags[0]));

    verticalHeader()->setDefaultSectionSize( 80 );
    horizontalHeader()->setDefaultSectionSize( 200 );

    int col = 0;
    model_->setHeaderData( col++, Qt::Horizontal, "SMILES" );
    model_->setHeaderData( col++, Qt::Horizontal, "Structure" );
    model_->setHeaderData( col++, Qt::Horizontal, "Formula" );
    model_->setHeaderData( col++, Qt::Horizontal, "Mass" );
    for ( auto tag: tags )
        model_->setHeaderData( col++, Qt::Horizontal, tag );        

    adcontrols::ChemicalFormula cformula;    

    do { // add PFTBA as demonstration
        model_->setRowCount( 1 );
        std::string smiles = "C(C(C(F)(F)F)(F)F)(C(N(C(C(C(C(F)(F)F)(F)F)(F)F)(F)F)C(C(C(C(F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F";
        RDKit::RWMol * mol = RDKit::SmilesToMol( smiles );
        
        model_->setData( model_->index( 0, 0 ), smiles.c_str() );
        do { // SVG
            std::string svg = adchem::drawing::toSVG( *mol );
            model_->setData( model_->index( 0, 1 ), QByteArray( svg.data(), static_cast<int>(svg.size()) ) );
            model_->item( 0, 1 )->setEditable( false );
        } while(0);
        do { // formula
            std::string formula = RDKit::Descriptors::calcMolFormula( *mol, true, false );
            model_->setData( model_->index( 0, 2 ), formula.c_str() );
            model_->setData( model_->index( 0, 3 ), cformula.getMonoIsotopicMass( formula ) );
        } while(0);
        delete mol;
    } while (0);    // end of PFTBA

    do {
        int row = model_->rowCount();
        model_->setRowCount( model_->rowCount() + static_cast<int>(adprot::AminoAcid::size()) );
        for ( adprot::AminoAcid::iterator it = adprot::AminoAcid::begin(); it != adprot::AminoAcid::end(); ++it ) {
            model_->setData( model_->index( row, 0 ), it->smiles() );
            RDKit::RWMol * mol = RDKit::SmilesToMol( it->smiles() );
            do { // SVG
                std::string svg = adchem::drawing::toSVG( *mol );
                model_->setData( model_->index( row, 1 ), QByteArray( svg.data(), static_cast<int>( svg.size() ) ) );
                model_->item( row, 1 )->setEditable( false );
            } while(0);
            std::string aa = adcontrols::ChemicalFormula::standardFormula( std::string( it->formula(false) ) + "H2O" );
			model_->setData( model_->index( row, 2 ), aa.c_str() );
            model_->setData( model_->index( row, 3 ), cformula.getMonoIsotopicMass( it->formula() ) );
            model_->setData( model_->index( row, 4 ), it->symbol() );

            std::string formula = RDKit::Descriptors::calcMolFormula( *mol, true, false );
            assert( formula == aa );
            model_->setData( model_->index( row, 5 ), formula.c_str() );

            ++row;
        }
        
    } while(0);

    for ( int col = 3; col < model_->columnCount(); ++col )
        resizeColumnToContents( col );
}

void
MolTableView::setMol( adchem::SDFile& file, QProgressBar& progressBar )
{
    if ( file ) {
        adcontrols::ChemicalFormula cformula;

        qtwrapper::waitCursor wait;

        //RDKit::SDMolSupplier& supplier = file.molSupplier();
		model_->setRowCount( static_cast<int>(file.size()) );

        progressBar.setRange( 0, static_cast<int>(file.size()) );
        progressBar.setVisible( true );
        progressBar.setTextVisible( true );

		uint32_t idx = 0;
		for ( auto mol: file ) {
            progressBar.setValue( idx + 1 );
            try {
                int col = 0;
                std::string smiles = RDKit::MolToSmiles( mol );
                if ( ! smiles.empty() ) {
                    model_->setData( model_->index( idx, col++ ), smiles.c_str() );
                    
                    // SVG
                    std::string svg = adchem::drawing::toSVG( mol );
                    model_->setData( model_->index( idx, col ), QByteArray( svg.data(), static_cast<int>(svg.size()) ) );
                    model_->item( idx, col )->setEditable( false );
                }
                col = 2;
                try {
                    mol.updatePropertyCache( false );
                    std::string formula = RDKit::Descriptors::calcMolFormula( mol, true, false );
                    model_->setData( model_->index( idx, col++ ), QString::fromStdString( formula) );
                    model_->setData( model_->index( idx, col++), cformula.getMonoIsotopicMass( formula ) );
                } catch ( std::exception& ex ) {
                    ADERROR() << ex.what();
                }
                col = 4;
                // associated data
                std::map< std::string, std::string > data;
                adchem::SDFile::iterator it = file.begin() + idx;
                if ( adchem::SDFile::parseItemText( it.itemText(), data ) ) {
                    for ( auto tag: tags ) {
                        auto it = data.find( tag );
                        if ( it != data.end() )
                            model_->setData( model_->index( idx, col ), it->second.c_str() );
                        ++col;
                    }
				}
				if ( idx == 10 )
					this->update();
            } catch ( std::exception& ex ) {
                ADERROR() << boost::current_exception_diagnostic_information() << ex.what();
            } catch ( ... ) {
                ADERROR() << boost::current_exception_diagnostic_information();
            }
            ++idx;
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

