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

#include "proteinwnd.hpp"
#include "proteintable.hpp"
#include "digestedpeptidetable.hpp"
#include "mainwindow.hpp"
#include <adplot/spectrumwidget.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/molecule.hpp>
#include <adextension/ipeptidehandler.hpp>
#include <adprot/protfile.hpp>
#include <adprot/protease.hpp>
#include <adprot/peptide.hpp>
#include <adprot/peptides.hpp>
#include <adprot/digestedpeptides.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <coreplugin/minisplitter.h>
#include <extensionsystem/pluginmanager.h>
#include <QVBoxLayout>
#include <QTextEdit>
#include <set>

using namespace peptide;

ProteinWnd::ProteinWnd(QWidget *parent) : QWidget(parent)
                                        , spectrum_( std::make_shared< adcontrols::MassSpectrum >() )
                                        , spectrumWidget_(0)
                                        , peptideTable_(0)
                                        , proteinTable_(0)
{
    init();
}

void
ProteinWnd::init()
{
    if ( QBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin( 0 );
        layout->setSpacing( 0 );
    
        if ( Core::MiniSplitter * splitter = new Core::MiniSplitter ) {  // protein | spectrum
            
            proteinTable_ = new ProteinTable;
            splitter->addWidget( proteinTable_ );

            if ( Core::MiniSplitter * splitter2 = new Core::MiniSplitter ) {
                splitter->addWidget( splitter2 );

                if ( ( peptideTable_ = new DigestedPeptideTable ) ) {
                    peptideTable_->setData( MainWindow::instance()->getChemicalFormula() );
                    splitter2->addWidget( peptideTable_ );
                }
                if ( ( spectrumWidget_ = new adplot::SpectrumWidget ) ) {
                    splitter2->addWidget( spectrumWidget_ );
                }
                splitter2->setOrientation( Qt::Vertical );
            }

            splitter->setOrientation( Qt::Horizontal );
            layout->addWidget( splitter );

			spectrumWidget_->setKeepZoomed( false );
            connect( proteinTable_, SIGNAL( selectionChanged( const QVector<int>& ) ), this, SLOT( handleSelectionChanged( const QVector<int>& ) ) );
            connect( peptideTable_, SIGNAL( selectedFormulae( const QVector<QString>& ) ), this, SLOT( handleFormulaeSelected( const QVector<QString>& ) ) );
        }
    }
}

void
ProteinWnd::setData( const adprot::protfile& file )
{
	proteinTable_->setData( file );
	proteinTable_->clearSelection();
	proteinTable_->selectRow( 0 );
}

void
ProteinWnd::handleFormulaeSelected( const QVector< QString >& formulae )
{
    qtwrapper::waitCursor wait;

    auto formulaParser = MainWindow::instance()->getChemicalFormula();
    double electron = formulaParser->getElectronMass();

    adcontrols::isotopeCluster isocalc;
    spectrum_->resize(0);
    
    for ( auto& formula: formulae ) {
        int charge(0);
        adcontrols::mol::molecule mol;
        if ( adcontrols::ChemicalFormula::getComposition( mol.elements, formula.toStdString() + "H", charge ) ) { // protenated
            isocalc( mol, 0 );
            double pmax = std::max_element( mol.cluster.begin(), mol.cluster.end()
                                            , [](const adcontrols::mol::isotope& a, const adcontrols::mol::isotope& b){
                                                return a.abundance < b.abundance;} )->abundance;
            auto last = std::remove_if( mol.cluster.begin(), mol.cluster.end(), [=]( const adcontrols::mol::isotope& i ){
                    return i.abundance / pmax < 0.001;}); // delete if peak high is less than 0.1% of base peak
            for ( auto pi = mol.cluster.begin(); pi != last; ++pi ) {
                *(spectrum_) << std::make_pair( pi->mass - electron, pi->abundance / pmax * 10000 ); // assume positive ion
            }
        }
    }

    spectrum_->setCentroid( adcontrols::CentroidNative );
    adcontrols::annotations& annots = spectrum_->get_annotations();
    annots.clear();
	double lMass = spectrum_->getMass( 0 );
	double hMass = spectrum_->getMass( spectrum_->size() - 1 );
    spectrum_->setAcquisitionMassRange( double( int( lMass / 10 ) * 10 ), double( int( ( hMass + 10 ) / 10 ) * 10 ) );
	spectrumWidget_->setAutoAnnotation( true );
    spectrumWidget_->setData( spectrum_, 0 );
}

void
ProteinWnd::handleSelectionChanged( const QVector<int>& rows )
{
    qtwrapper::waitCursor wait;

    if ( rows.isEmpty() )
        return;

	if ( rows.size() == 1 ) {
		protSelChanged( rows[0] );
		return;
	}

    auto formulaParser = MainWindow::instance()->getChemicalFormula();
    auto enzyme = MainWindow::instance()->get_protease();

    if ( std::shared_ptr< adprot::protfile > ptr = MainWindow::instance()->get_protfile() ) {

        std::set< std::string > digested_peptides;
        for ( auto row: rows ) {

            std::string protein_sequence;
            if ( proteinTable_->getSequence( row, protein_sequence ) ) {

                std::vector< std::string > output;
                adprot::protease::digest( *enzyme, protein_sequence, output );
                for ( auto& sequence: output )
                    digested_peptides.insert( sequence );
            }
        }

        adprot::digestedPeptides digested( adprot::protein(), *enzyme );

        for ( const auto& sequence: digested_peptides ) {
            std::string stdFormula = adcontrols::ChemicalFormula::standardFormula( adprot::peptide::formula( sequence ) );
            double mass = formulaParser->getMonoIsotopicMass( stdFormula );
            digested << adprot::peptide( sequence, stdFormula, mass );
        }
        // order of protein sequence
        peptideTable_->setData( digested );
        //sort_and_unique( vec );
        //setData( vec );
    }
}


void
ProteinWnd::protSelChanged( int row )
{
    auto formulaParser = MainWindow::instance()->getChemicalFormula();

    if ( std::shared_ptr< adprot::protfile > ptr = MainWindow::instance()->get_protfile() ) {
        if ( row < int( ptr->size() ) ) {

            auto it = ptr->begin() + row;

            if ( auto enzyme = MainWindow::instance()->get_protease() ) {
                
                adprot::digestedPeptides digested( *it, *enzyme );
                            
                std::vector< std::string > sequences;
                adprot::protease::digest( *enzyme, it->sequence(), sequences );
                std::vector< peptide_formula_mass_type > vec;
                for ( const auto& sequence: sequences ) {

                    std::string stdFormula = adcontrols::ChemicalFormula::standardFormula( adprot::peptide::formula( sequence ) );

                    double mass = formulaParser->getMonoIsotopicMass( stdFormula );
                    digested << adprot::peptide( sequence, stdFormula, mass );

                    vec.push_back( std::make_tuple( sequence // amino acid sequence
                                                    , stdFormula // chemical formula (free for N, C terminals, normal H2O)
                                                    , mass // neutral mass
                                       ) );
                }
                // order of protein sequence
                peptideTable_->setData( digested );

                adprot::peptides peptides = digested.peptides(); // deep copy

                // remove same peptides for spectrum draw
                sort_and_unique( peptides );
                setData( peptides );

                for( auto handler: ExtensionSystem::PluginManager::instance()->getObjects< adextension::iPeptideHandler >() ) {
                    handler->onProteinSelected( digested );
                }
            }
        }
    }
}

void
//ProteinWnd::sort_and_unique( std::vector< peptide_formula_mass_type >& peptides )
ProteinWnd::sort_and_unique( adprot::peptides& peptides )
{
    std::sort( peptides.begin(), peptides.end(), []( const adprot::peptide& lhs, const adprot::peptide& rhs ){
            return lhs.mass() < rhs.mass();
        });
    auto it =
        std::unique( peptides.begin(), peptides.end(), []( const adprot::peptide& lhs, const adprot::peptide& rhs ){
                return lhs.sequence() == rhs.sequence();
            });
    peptides.erase( it, peptides.end() );
}

void
ProteinWnd::setData( const adprot::peptides& peptides )
{
    if ( auto formulaParser = MainWindow::instance()->getChemicalFormula() ) {
        
        spectrum_->resize( 0 );
		spectrum_->setCentroid( adcontrols::CentroidNative );
        adcontrols::annotations& annots = spectrum_->get_annotations();
		annots.clear();
        
        auto it = std::max_element( peptides.begin(), peptides.end()
                                    , [](const adprot::peptide& lhs, const adprot::peptide& rhs){ return lhs.mass() < rhs.mass(); });
		double hMass = it->mass();

        spectrum_->setAcquisitionMassRange( 100, double( int( ( hMass + 500 ) / 500 ) * 500 ) );
        
        double proton = formulaParser->getMonoIsotopicMass( "H" ) - formulaParser->getElectronMass();

        int idx = 0;
        for ( auto& peptide: peptides ) {
            double mass = peptide.mass() + proton; // M+H+
            double h = 10 + ( idx % 16 ) * 10;
            (*spectrum_) << std::make_pair( mass, h );
            annots << adcontrols::annotation( peptide.sequence(), mass, h, idx, 0 );
            ++idx;
        }
        spectrumWidget_->setAutoAnnotation( false );
        spectrumWidget_->setData( spectrum_, 0 );
    }    
}

