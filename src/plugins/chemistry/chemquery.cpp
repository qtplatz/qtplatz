/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "chemquery.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adchem/drawing.hpp>
#include <adfs/sqlite.hpp>
#include <adfs/sqlite3.h>
#include <adportable/debug.hpp>
#include <QByteArray>
#include <QObject>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <INCHI-API/inchi.h>

using namespace chemistry;

ChemQuery::ChemQuery(adfs::sqlite& db) : sql_( db )
{
}

ChemQuery::ChemQuery( const ChemQuery& t ) : std::enable_shared_from_this< ChemQuery >( t )
                                           , sql_( t.sql_ )
{
}

bool
ChemQuery::prepare( const std::wstring& sql )
{
    sql_.reset();
    return sql_.prepare( sql );
}

bool
ChemQuery::prepare( const std::string& sql )
{
    sql_.reset();
    return sql_.prepare( sql );
}

adfs::sqlite_state
ChemQuery::step()
{
    state_ = sql_.step();
    return state_;
}

size_t
ChemQuery::column_count() const
{
    return sql_.column_count();
}

QString
ChemQuery::column_name( size_t idx ) const
{
    std::string name = sql_.column_name( int( idx ) );
    return QString::fromStdString( name );
}

QString
ChemQuery::column_name_tr( const QString& d )
{
    struct column_name {
        const QString loc_c;
        const QString i10n;
    };
    column_name names[] = {
        { "smiles", QObject::tr( "SMILES" ) }
        , { "formula", QObject::tr( "Formula" ) }
        , { "mass", QObject::tr( "Mass" ) }
        , { "svg", QObject::tr( "Structure" ) }
        , { "synonym", QObject::tr( "Synonym" ) }
        , { "casrn", QObject::tr( "CAS NO." ) }
        , { "SystematicName", QObject::tr( "Systematic Name" ) }
    };
    
    for ( auto& t : names ) {
        if ( t.loc_c == d )
            return t.i10n;
    }
    return d;
}


QVariant
ChemQuery::column_value( size_t idx ) const
{
    switch ( sql_.column_type( int( idx ) ) ) {
    case SQLITE_INTEGER: return QVariant( static_cast< qlonglong >( sql_.get_column_value< int64_t >( int( idx ) ) ) );
    case SQLITE_FLOAT:   return QVariant( sql_.get_column_value< double >( int( idx ) ) );
    case SQLITE_TEXT:    return QVariant( QString( sql_.get_column_value< std::string >( int( idx ) ).c_str() ) );
    case SQLITE_BLOB: {
        if ( sql_.column_name( int( idx ) ) == "svg" ) {
            auto blob = sql_.get_column_value < adfs::blob >( int( idx ) );
            return QByteArray( reinterpret_cast<const char *>( blob.data() ), int( blob.size() ) );
        }
    }
    case SQLITE_NULL:    return QVariant();
    }
    return QVariant();
}

bool
ChemQuery::insert( const RDKit::ROMol& mol, const std::string& smiles, const std::string& synonym, const std::string& _inchi )
{
    std::string inchi( _inchi );
#if !(defined WIN32 && defined _DEBUG) // it's too slow on debug mode vc14
    if ( inchi.empty() ) {
        RDKit::ExtraInchiReturnValues rv;
        inchi = RDKit::MolToInchi( mol, rv );
    }
#endif

    if ( sql_.prepare( "SELECT COUNT(*) FROM mols WHERE InChI = ?" ) ) {
        sql_.bind( 1 ) = inchi;
        if ( sql_.step() == adfs::sqlite_row ) {
            if ( sql_.get_column_value< int64_t >( 0 ) > 0 ) {
                return false; // duplicate
            }
        }
    }

    std::string svg = adchem::drawing::toSVG( mol );
    std::string formula = RDKit::Descriptors::calcMolFormula( mol, true, false );
    double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( formula );
    std::string inchikey = RDKit::InchiToInchiKey( inchi );

    if ( sql_.prepare( "INSERT OR REPLACE INTO mols (smiles,svg,formula,mass,inchi,inchikey) VALUES(?,?,?,?,?,?)" ) ) {

        int row(1);
        sql_.bind( row++ ) = smiles;
        sql_.bind( row++ ) = adfs::blob( svg.size(), svg.data() );
        sql_.bind( row++ ) = formula;
        sql_.bind( row++ ) = mass;
        sql_.bind( row++ ) = inchi;
        sql_.bind( row++ ) = inchikey;
        
        if ( sql_.step() == adfs::sqlite_done ) {
            row = 1;
            sql_.prepare( "INSERT INTO synonyms (id,synonym) SELECT id, ? FROM mols WHERE inchi = ?" );
            sql_.bind( row++ ) = synonym;
            sql_.bind( row++ ) = inchi;
            return sql_.step();
        }
    }
    return false;
}

// bool
// ChemQuery::insert_mol( const std::string& smiles
//                        , const std::string& svg
//                        , const std::string& formula
//                        , double mass
//                        , const std::string& synonym
//                        , const std::string& inchi, const std::string& inchikey )
// {
//     if ( sql_.prepare( "INSERT OR REPLACE INTO mols (smiles,svg,formula,mass,inchi,inchikey) VALUES(?,?,?,?,?,?)" ) ) {

//         // auto uuid = generator( smiles );
//         int row(1);
//         sql_.bind( row++ ) = smiles;
//         sql_.bind( row++ ) = adfs::blob( svg.size(), svg.data() );
//         sql_.bind( row++ ) = formula;
//         sql_.bind( row++ ) = mass;
//         sql_.bind( row++ ) = inchi;
//         sql_.bind( row++ ) = inchikey;
        
//         if ( sql_.step() == adfs::sqlite_done ) {
//             sql_.prepare( "INSERT INTO synonyms (id,synonym) SELECT id, ? FROM mols WHERE inchi = ?" );
//             sql_.bind( 1 ) = synonym;
//             sql_.bind( 2 ) = inchi;
//             return sql_.step();
//         }
        
//     }
//     return false;
// }

