/**************************************************************************
** Copyright (C) 2022-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include "sqlexport.hpp"
#include "ionreaction.hpp"
#include <adcontrols/constants.hpp>
#include <adcontrols/isocluster.hpp>
#include <adcontrols/ionreactionmethod.hpp>
#include <adcontrols/make_combination.hpp>
#include <adcontrols/molecule.hpp>
#include <adfs/get_column_values.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>

namespace lipidid {

    class SQLExport::impl {
        impl( const impl& ) = delete;
        impl& operator = ( const impl& ) = delete;
    public:
        impl() {}
        impl( const std::filesystem::path& path ) : path_( path ) {}
        ~impl() {}

        std::unique_ptr< adfs::sqlite > sqlite_;
        std::filesystem::path path_;
        adcontrols::IonReactionMethod method_;
    };

}

using namespace lipidid;

SQLExport::SQLExport() : impl_( std::make_unique< impl >() )
{
}

SQLExport::SQLExport( adcontrols::IonReactionMethod&& t ) : impl_( std::make_unique< impl >() )
{
    impl_->method_ = std::move( t );
}

SQLExport::~SQLExport()
{
}

bool
SQLExport::create_database( const std::filesystem::path& file )
{
    if ( std::filesystem::exists( file ) ) {
        auto ofile = file.string() + std::string( ".bak" );
        if ( std::filesystem::exists( ofile ) )
            std::filesystem::remove( ofile );
        std::filesystem::rename( file, ofile );
    }

    if ( auto sqlite = std::make_unique< adfs::sqlite >() ) {
        if ( sqlite->open( file.string().c_str(), adfs::opencreate ) ) {
            impl_->sqlite_ = std::move( sqlite );

            auto sql = adfs::stmt( *impl_->sqlite_ );
            sql.exec( "PRAGMA synchronous = OFF" );
            sql.exec( "PRAGMA journal_mode = MEMORY" );
            sql.exec( "PRAGMA FOREIGN_KEYS = ON" );

            return create_tables();
        }
    }
    return false;
}

bool
SQLExport::create_tables()
{
    if ( impl_->sqlite_ ) {
        auto sql = adfs::stmt( *impl_->sqlite_ );
        sql.exec(
            "CREATE TABLE IF NOT EXISTS mols ("
            "id INTEGER PRIMARY KEY"
            ",formula          TEXT"
            ",mass             REAL"
            ",SlogP            REAL"
            ",inchiKey         TEXT"
            ",SMILES           TEXT"
            ",UNIQUE (id,inchiKey)"
            ")"
            );
        sql.exec(
            "CREATE TABLE IF NOT EXISTS ion ("
            "id INTEGER PRIMARY KEY"
            ", molid            INTEGER"
            ", i8n              TEXT"
            ", polarity         INTEGER"
            ", formula          TEXT"
            ", charge           INTEGER"
            ", mass             REAL"
            ", FOREIGN KEY ( molid ) REFERENCES mols ( id )"
            ")"
            );
        sql.exec(
            "CREATE TABLE IF NOT EXISTS isotope ("
            "id INTEGER PRIMARY KEY"
            ", ionid            INTEGER"
            ", mass             REAL"
            ", abundance        REAL"
            ", FOREIGN KEY ( ionid ) REFERENCES ion ( id )"
            ")"
            );
        return true;
    }
    return false;
}

bool
SQLExport::drop_tables()
{
    if ( impl_->sqlite_ ) {
        auto sql = adfs::stmt( *impl_->sqlite_ );
        sql.exec( "DROP TABLE IF EXISTS isotope" );
        sql.exec( "DROP TABLE IF EXISTS ion" );
        sql.exec( "DROP TABLE IF EXISTS mols" );
        return true;
    }
    return false;
}

// bool
// SQLExport::export_ion_reactions(  const std::vector< std::vector< adcontrols::lipidid::molecule_pair_t > >& list
//                                   , adcontrols::ion_polarity polarity
//                                   , bool testing )
bool
SQLExport::export_ion_reactions( adcontrols::IonReactionMethod&& t
                                 , bool testing )
{
    adcontrols::isoCluster cluster{ 0.001, 5000 };

    const std::string SQL = testing ?
        "SELECT id,formula,mass,SlogP,inchiKey,smiles FROM mols WHERE mass > 200 ORDER by mass LIMIT 10"
        : "SELECT id,formula,mass,SlogP,inchiKey,smiles FROM mols ORDER by mass";

    impl_->method_ = std::move( t );
    auto pos_list = adcontrols::lipidid::make_combination( impl_->method_, adcontrols::polarity_positive );
    auto neg_list = adcontrols::lipidid::make_combination( impl_->method_, adcontrols::polarity_negative );

    auto ssql = adfs::stmt( *document::instance()->sqlite() );
    auto sql  = adfs::stmt( *impl_->sqlite_ );

    int64_t total(0);
    if ( ssql.prepare( "SELECT COUNT(*) FROM mols" ) ) {
        if ( ssql.step() == adfs::sqlite_row ) {
            total = ssql.get_column_value< int64_t >( 0 );
        }
    }

    int64_t count(0);
    if ( ssql.prepare( SQL ) ) {
        while ( ssql.step() == adfs::sqlite_row ) {
            //                                id,      formula,     mass,   SlogP,  inchiKey, SMILES
            auto t = adfs::get_column_values< int64_t, std::string, double, double, std::string, std::string >( ssql );
            ADDEBUG() << t << "\t" << count++ << "/" << total;

            sql.prepare( "INSERT OR IGNORE INTO mols (id,formula,mass,SlogP,inchiKey,smiles) VALUES (?,?,?,?,?,?)" );
            sql.bind( 1 ) = std::get< 0 >( t ); // id
            sql.bind( 2 ) = std::get< 1 >( t ); // formula
            sql.bind( 3 ) = std::get< 2 >( t ); // mass
            sql.bind( 4 ) = std::get< 3 >( t ); // SlogP
            sql.bind( 5 ) = std::get< 4 >( t ); // inchiKey
            sql.bind( 6 ) = std::get< 5 >( t ); // smiles

            if ( sql.step() == adfs::sqlite_done ) {

                IonReaction rxn( std::get< 1 >( t ) ); // formula
                for ( const auto& alist: pos_list ) {
                    auto mol = rxn.make_molecule( alist ); // apply an add/lose
                    cluster( mol, mol.charge() );
                    // ADDEBUG() << "\tpos: " << rxn.mass( alist ) << "\t" << adcontrols::lipidid::to_string( alist );
                    insert_molecule( std::get< 0 >( t ), mol, adcontrols::polarity_positive );
                }
                for ( const auto& alist: neg_list ) {
                    auto mol = rxn.make_molecule( alist ); // apply an add/lose
                    cluster( mol, mol.charge() );
                    // ADDEBUG() << "\tneg: " << rxn.mass( alist ) << "\t" << adcontrols::lipidid::to_string( alist );
                    insert_molecule( std::get< 0 >( t ), mol, adcontrols::polarity_negative );
                }
            } else {
                ADDEBUG() << sql.errmsg();
            }
        }
    }
    return true;
}

bool
SQLExport::insert_molecule( int64_t molid
                            , const adcontrols::mol::molecule& mol
                            , adcontrols::ion_polarity polarity ) const
{
    auto sql = adfs::stmt( *impl_->sqlite_ );

    sql.prepare( "INSERT INTO ion (molid,i8n,polarity,formula,charge,mass) VALUES (?,?,?,?,?,?)" );
    sql.bind( 1 ) = molid;
    sql.bind( 2 ) = impl_->method_.i8n();
    sql.bind( 3 ) = static_cast< int >( polarity );
    sql.bind( 4 ) = mol.formula();
    sql.bind( 5 ) = mol.charge();
    sql.bind( 6 ) = mol.mass();

    if ( sql.step() == adfs::sqlite_done ) {

        const int64_t ionid = impl_->sqlite_->last_insert_rowid();

        sql.prepare( "INSERT INTO isotope (ionid,mass,abundance) VALUES (?,?,?)" );
        for ( const auto& i: mol.cluster() ) {
            sql.bind( 1 ) = ionid;
            sql.bind( 2 ) = impl_->method_.i8n();
            sql.bind( 3 ) = static_cast< int >( polarity );
            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << sql.errmsg();
        }
    } else {
        ADDEBUG() << sql.errmsg();
    }

    return true;
}
