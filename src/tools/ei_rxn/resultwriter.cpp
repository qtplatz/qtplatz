// -*- C++ -*-
/**************************************************************************
**
** MIT License
** Copyright (c) 2021-2022 Toshinobu Hondo, Ph.D

** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:

** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**************************************************************************/

#include "resultwriter.hpp"
#include "product_record.hpp"
#include <filesystem>
#include <adfs/sqlite.hpp>
#include <adutils/datafile_signature.hpp>
#include <adportable/debug.hpp>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/ROMol.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/inchi.h>

namespace {

    void createTable( adfs::sqlite& db ) {
        adfs::stmt sql( db );

        sql.exec(
            "CREATE TABLE IF NOT EXISTS analyte ( "
            "id INTEGER PRIMARY KEY "
            ", canonical_smiles TEXT NOT NULL UNIQUE"
            ", formula TEXT"
            ", exact_mass REAL"
            ", comment TEXT"
            ")"
            );

        sql.exec(
            "CREATE TABLE IF NOT EXISTS synonym ( "
            "id INTEGER PRIMARY KEY"
            ", analyte_id INTEGER NOT NULL"
            ", name TEXT NOT NULL"
            ", name_type TEXT"
            ", UNIQUE(analyte_id, name)"
            ", FOREIGN KEY(analyte_id) REFERENCES analyte(id)"
            ")"
            );

        sql.exec(
            "CREATE TABLE IF NOT EXISTS product ( "
            "id INTEGER PRIMARY KEY"
            ", analyte_id INTEGER NOT NULL"
            ", canonical_smiles TEXT NOT NULL"
            ", formula TEXT"
            ", exact_mass REAL NOT NULL"
            ", charge INTEGER"
            ", origins TEXT"
            ", comment TEXT"
            ", UNIQUE(analyte_id, canonical_smiles)"
            ", FOREIGN KEY(analyte_id) REFERENCES analyte(id)"
            ")"
            );

        sql.exec(
            "CREATE INDEX idx_product_mass "
            "ON product(exact_mass)" );

        sql.exec(
            "CREATE INDEX idx_product_analyte "
            "ON product(analyte_id)" );
    }

    const static boost::uuids::uuid __writer_uuid__ = {
        0x96, 0xD0, 0x27, 0x46, 0x91, 0x7D, 0x44, 0xCF
        , 0x9C, 0xF1, 0x75, 0x99, 0xEC, 0x71, 0x01, 0x61
    };
} // namespace


class resultWriter::impl {
public:
    ~impl() {}
    impl() : db_( std::make_shared< adfs::sqlite >() ) {
    }
    std::shared_ptr< adfs::sqlite > db_;
};

resultWriter::~resultWriter()
{
}

resultWriter::resultWriter() : impl_( std::make_unique< impl >() )
{
}

bool
resultWriter::open( const std::filesystem::path& path ) const
{
    if ( path.has_extension() && path.extension() == ".db" ) {
        if ( impl_->db_->open( path.c_str() ) ) {
            using namespace adutils::data_signature;
            adfs::stmt sql( *impl_->db_ );
            sql.exec( "PRAGMA FOREIGN_KEYS = ON" );
            if ( not is_table_exists( *impl_->db_, "datafile_signature" ) ) {
                datafileSignature::create_table( *impl_->db_ );
                sql << datum_t{ "creator", value_t( __writer_uuid__ ) };
                createTable( *impl_->db_ );
            }
            std::map< std::string, value_t > sigs;
            sql >> sigs;
            if ( sigs.find( "creator" ) != sigs.end() ) {
                if ( std::get< boost::uuids::uuid >( sigs[ "creator" ] ) == __writer_uuid__ ) {
                    return true;
                } else {
                    ADDEBUG() << "Data file is not created by this object";
                    return false;
                }
            }
        }
    }
    return false;
}

bool
resultWriter::write( const RDKit::ROMol& analyte
                     , const std::vector< std::string >& synonyms
                     , const std::map< std::string, product_record >& unique ) const
{
    adfs::stmt sql( *impl_->db_ );
    const auto key = RDKit::MolToSmiles( analyte, true );
    ADDEBUG() << key;
    // sql.prepare( "INSERT INTO analyte "
    //              "(canonical_smiles,formula,exact_mass,comment) VALUES (?,?,?,?) "
    //              "ON CONFLICT(canonical_smiles) DO UPDATE SET "
    //              "formula     = excluded.formula"
    //              ",exact_mass = excluded.exact_mass"
    //              ",comment    = excluded.comment" );
    sql.prepare( "INSERT OR REPLACE INTO analyte "
                 "(canonical_smiles,formula,exact_mass,comment) VALUES (?,?,?,?)" );
    sql.bind( 1 ) = key;
    sql.bind( 2 ) = RDKit::Descriptors::calcMolFormula( analyte );
    sql.bind( 3 ) = RDKit::Descriptors::calcExactMW( analyte );
    sql.bind( 4 ) = std::string(); // comment
    if ( sql.step() != adfs::sqlite_done )
        ADDEBUG() << sql.errmsg();
    sql.reset();


    // sql.prepare( "select * from analyte" );
    // while ( sql.step() == adfs::sqlite_row ) {
    //     ADDEBUG() << sql.get_column_value< int64_t >(0); // id
    //     ADDEBUG() << sql.get_column_value< std::string >(1); // key (smiles)
    // }
    //---

    sql.prepare( "INSERT OR REPLACE INTO synonym"
                 "(analyte_id,name) VALUES "
                 "((SELECT id FROM analyte WHERE canonical_smiles=?), ?)" );
    for ( const auto& synonym: synonyms ) {
        sql.bind( 1 ) = key;
        sql.bind( 2 ) = synonym;
        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << sql.errmsg();
        sql.reset();
    }

    //--------
    sql.prepare( "INSERT OR REPLACE INTO product "
                 "(analyte_id,canonical_smiles,formula,exact_mass,charge,origins,comment) VALUES "
                 "((SELECT id FROM analyte WHERE canonical_smiles=?),?,?,?,?,?,?)" );

    for ( const auto& [name, product]: unique ) {
        std::ostringstream origins;
        for ( const auto& origin: product.origins ) {
            origins << origin << ";";
        }

        sql.bind( 1 ) = key;
        sql.bind( 2 ) = RDKit::MolToSmiles( *product.mol, true );
        sql.bind( 3 ) = RDKit::Descriptors::calcMolFormula( *product.mol );
        sql.bind( 4 ) = RDKit::Descriptors::calcExactMW( *product.mol ); // neutral mass
        sql.bind( 5 ) = RDKit::MolOps::getFormalCharge( *product.mol );
        sql.bind( 6 ) = origins.str();
        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << sql.errmsg();
        sql.reset();
    }


    return false;
}
