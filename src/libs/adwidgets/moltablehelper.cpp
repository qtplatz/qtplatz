/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "moltablehelper.hpp"
#include <adchem/mol.hpp>
#include <adchem/sdmolsupplier.hpp>
#include <adchem/smilestosvg.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/optional.hpp>
#include <QByteArray>
#include <QClipboard>
#include <QString>
#include <QUrl>
#include <tuple>

using namespace adwidgets;

adportable::optional< std::tuple< QString, QByteArray > > // formula, svg
MolTableHelper::SmilesToSVG::operator()( const QString& smiles ) const
{
    if ( auto d = adchem::SmilesToSVG()( smiles.toStdString() ) ) {
#if __cplusplus >= 201703L
        auto [ formula, svg ] = *d;
#else
        std::string formula, svg;
        std::tie( formula, svg ) = *d;
#endif
        return std::make_tuple( QString::fromStdString( formula ), QByteArray( svg.data(), svg.size() ) );
    }
    return {};
}

//typedef std::tuple< QString, QString, QByteArray > value_type; // formula,smiles,svg

std::vector< MolTableHelper::SDMolSupplier::value_type >
MolTableHelper::SDMolSupplier::operator()( const QUrl& url ) const
{
    std::vector< MolTableHelper::SDMolSupplier::value_type > results;

    adchem::SDMolSupplier supplier( url.toLocalFile().toStdString() );

    for ( size_t i = 0; i < supplier.size(); ++i ) {
#if __cplusplus >= 201703L
        auto [ formula, smiles, svg ] = supplier[ i ];
#else
        std::string formula, smiles, svg;
        std::tie( formula, smiles, svg ) = supplier[ i ];
#endif
        results.emplace_back( QString::fromStdString( formula ), QString::fromStdString( smiles ), QByteArray( svg.data(), svg.size() ) );
    }
    return results;
}


std::vector< MolTableHelper::SDMolSupplier::value_type >
MolTableHelper::SDMolSupplier::operator()( const QClipboard* clipboard ) const
{
    if ( clipboard == nullptr )
        return {};

    std::vector< MolTableHelper::SDMolSupplier::value_type > results;
    adchem::SDMolSupplier supplier;
    supplier.setData( clipboard->text().toStdString() );

    for ( size_t i = 0; i < supplier.size(); ++i ) {
#if __cplusplus >= 201703L
        auto [ formula, smiles, svg ] = supplier[ i ];
#else
        std::string formula, smiles, svg;
        std::tie( formula, smiles, svg ) = supplier[ i ];
#endif
        results.emplace_back( QString::fromStdString( formula ), QString::fromStdString( smiles ), QByteArray( svg.data(), svg.size() ) );
    }
    return results;
}


// static
double
MolTableHelper::monoIsotopicMass( const QString& formula, const QString& adducts )
{
    using adcontrols::ChemicalFormula;

    auto expr = formula;

    if ( ! adducts.isEmpty() )
        expr += " " + adducts;
    double exactMass = ChemicalFormula().getMonoIsotopicMass( ChemicalFormula::split( expr.toStdString() ) ).first;
    return exactMass;
}

//static
adportable::optional< std::pair< double, double > >
MolTableHelper::logP( const QString& smiles )
{
#if HAVE_RDKit
    auto mol = adchem::mol( smiles.toStdString() );
    return mol.logP();
#else
    return {};
#endif
}
