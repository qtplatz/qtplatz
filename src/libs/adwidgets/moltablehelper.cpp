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
#include "adducts_type.hpp"
#include <adchem/mol.hpp>
#include <adchem/sdmolsupplier.hpp>
#include <adchem/smilestosvg.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/moltable.hpp>
#include <adportable/optional.hpp>
#include <adportable/debug.hpp>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QMimeData>
#include <QString>
#include <QUrl>
#include <boost/json.hpp>
#include <boost/optional/optional_io.hpp>
#include <tuple>

namespace adwidgets {
    namespace moltable {
        std::tuple< double, QString >
        computeMass( const QString& formula, const QString& adducts )
        {
            std::string stdformula = formula.toStdString();
            std::string stdadducts = adducts.toStdString();
            auto v = adcontrols::ChemicalFormula::standardFormulae( stdformula, stdadducts );
            if ( v.empty() )
                return {0,{}};
            return { adcontrols::ChemicalFormula().getMonoIsotopicMass( v[0] ), QString::fromStdString( v[0] ) }; // handle first molecule
        }

    }
}

using namespace adwidgets;

adportable::optional< std::tuple< QString, QByteArray > > // formula, svg
MolTableHelper::SmilesToSVG::operator()( const QString& smiles ) const
{
    if ( auto d = adchem::SmilesToSVG()( smiles.toStdString() ) ) {
        auto formula = std::get<0>(*d);
        auto svg = std::get<1>(*d);
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
        std::string formula, smiles, svg;
        std::tie( formula, smiles, svg ) = supplier[ i ];
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
        std::string formula, smiles, svg;
        std::tie( formula, smiles, svg ) = supplier[ i ];
        results.emplace_back( QString::fromStdString( formula ), QString::fromStdString( smiles ), QByteArray( svg.data(), svg.size() ) );
    }
    return results;
}


// static
double
MolTableHelper::monoIsotopicMass( const QString& formula, const QString& adducts )
{
    return std::get< 0 >( moltable::computeMass( formula, adducts ) );
    // using adcontrols::ChemicalFormula;

    // auto expr = formula;

    // if ( ! adducts.isEmpty() )
    //     expr += " " + adducts;
    // double exactMass = ChemicalFormula().getMonoIsotopicMass( ChemicalFormula::split( expr.toStdString() ) ).first;
    // return exactMass;
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

adportable::optional< adcontrols::moltable >
MolTableHelper::paste()
{
    if ( auto md = QApplication::clipboard()->mimeData() ) {
        auto data = md->data( "application/json" );
        if ( data.isEmpty() ) {
            ADDEBUG() << "###### handlePaste -- text/plain #######";
            auto text = md->data( "text/plain" );
            if ( text.at( 0 ) == '{' ) { // check if json
                boost::system::error_code ec;
                auto jv = boost::json::parse( text.toStdString(), ec );
                if ( !ec ) {
                    auto mols = boost::json::value_to< adcontrols::moltable >( jv );
                    return mols;
                }
            } else {
                std::istringstream in( text.toStdString() );
                std::string line;
                adcontrols::moltable mols;
                while ( std::getline( in, line ) ) {
                    // ADDEBUG() << "\t" << line;
                    adcontrols::moltable::value_type value;
                    if (( value.mass() = adcontrols::ChemicalFormula().getMonoIsotopicMass( line ) > 0.1  )) {
                        value.formula() = line;
                        value.enable() = true;
                        value.abundance() = 1.0;
                        mols << value;
                        // ja.push_back( {{ "formula", line }, {"enable", true}, {"abundance", 1.0}} );
                    }
                }
                // jv = {{ "moltable", ja }};
                return mols;
            }
        } else {
            // ADDEBUG() << data.toStdString();
            boost::system::error_code ec;
            auto jv = boost::json::parse( data.toStdString(), ec );
            if ( !ec ) {
                auto mols = boost::json::value_to< adcontrols::moltable >( jv );
                return mols;
            }
        }
    }
    return {};
}

/////////////


namespace adwidgets {
    namespace moltable {

        template<> void __assign( moltable::col_formula&, const QModelIndex& index, adcontrols::moltable::value_type& value )
        {
            value.formula() = index.data().value< col_formula::value_type >().toStdString();
            value.enable() = index.data( Qt::CheckStateRole ).toBool();
        }

        template<> void __assign( moltable::col_adducts& t, const QModelIndex& index, adcontrols::moltable::value_type& value )
        {
            // ADDEBUG() << "col_adducts\t" << std::make_pair( index.row(), index.column() );
            auto v = index.data( Qt::UserRole + 1 );
            if ( v.canConvert< adducts_type >() )
                value.adducts_ = v.value< adducts_type >().adducts; // std::tuple< pos, neg >
            else
                value.adducts<adcontrols::polarity_positive>() = index.data().value< col_adducts::value_type >().toStdString(); // handle as positive ion
        }

        template<> void __assign( moltable::col_mass& t, const QModelIndex& index, adcontrols::moltable::value_type& value )
        {
            value.mass() = index.data().value< col_mass::value_type >();
        }

        template<> void __assign( moltable::col_retentionTime& t, const QModelIndex& index, adcontrols::moltable::value_type& value )
        {
            if ( index.data().isValid() && index.data().value< col_retentionTime::value_type >() > 0 )
                value.set_tR( index.data().value< col_retentionTime::value_type >() );
            else
                value.set_tR( {} );
        }

        template<> void __assign( moltable::col_msref& t, const QModelIndex& index, adcontrols::moltable::value_type& value )
        {
            value.setIsMSRef( index.data().value< col_msref::value_type >() );
        }

        template<> void __assign( moltable::col_protocol& t, const QModelIndex& index, adcontrols::moltable::value_type& value )
        {
            value.setProtocol( index.data().value< col_protocol::value_type >() );
        }

        template<> void __assign( moltable::col_synonym& t, const QModelIndex& index, adcontrols::moltable::value_type& value )
        {
            value.synonym() = index.data().value< col_synonym::value_type >().toStdString();
        }

        template<> void __assign( moltable::col_memo& t, const QModelIndex& index, adcontrols::moltable::value_type& value )
        {
            value.set_description( index.data().value< col_memo::value_type >().toStdWString() );
        }

        template<> void __assign( moltable::col_smiles& t, const QModelIndex& index, adcontrols::moltable::value_type& value )
        {
            value.smiles() = index.data().value< col_smiles::value_type >().toStdString();
        }

        template<> void __assign( moltable::col_abundance& t, const QModelIndex& index, adcontrols::moltable::value_type& value )
        {
            value.abundance() = index.data().value< col_abundance::value_type >();
        }

    }
}
