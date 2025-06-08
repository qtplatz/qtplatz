/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adportable/debug.hpp>
#include <GraphMol/DistGeomHelpers/Embedder.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/GeneralFileReader.h>
#include <GraphMol/FileParsers/MolWriters.h>
#include <GraphMol/GraphMol.h>
#include <GraphMol/ROMol.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/ForceFieldHelpers/UFF/UFF.h>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <ratio>

namespace po = boost::program_options;

enum package {
    MOPAC
    , ORCA
    , XYZ
};

namespace {

    class adduct {
        std::string adduct_;
        std::string nearby_;
        double bond_length_;
        RDGeom::Point3D pos_;
    public:
        adduct( const std::string& a, const std::string& nearby, double distance )
            : adduct_( a ), nearby_( nearby ), bond_length_( distance ) {
        }
        adduct( const adduct& t ) : adduct_( t.adduct_), nearby_( t.nearby_ ), bond_length_( t.bond_length_ ) {}

        std::string symbol() const { return adduct_; }
        std::string nearby() const { return nearby_; }
        RDGeom::Point3D pos( const RDKit::ROMol& mol ) const {
            return place( mol, nearby_, bond_length_ );
        }

    private:
        RDGeom::Point3D place( const RDKit::ROMol& mol, const std::string& nearby, double bond_length = 2.3) const {
            for ( size_t i = 0; i < mol.getNumAtoms(); ++i ) {
                auto atom = mol.getAtomWithIdx( i );
                if ( atom->getSymbol() == nearby ) {
                    auto com = compute_com( mol );
                    auto pos = mol.getConformer().getAtomPos( i );
                    double dx = com.x - pos.x;
                    double dy = com.y - pos.y;
                    double dz = com.z - pos.z;
                    double norm = std::sqrt(dx*dx + dy*dy + dz*dz);
                    dx /= norm; dy /= norm; dz /= norm;
                    return {pos.x + dx * bond_length, pos.y + dy * bond_length, pos.z + dz * bond_length};
                }
            }
            return {};
        }

        RDGeom::Point3D compute_com( const RDKit::ROMol& mol ) const {
            RDGeom::Point3D com;
            for ( size_t i = 0; i < mol.getNumAtoms(); ++i ) {
                auto atom = mol.getAtomWithIdx( i );
                auto pos = mol.getConformer().getAtomPos( i );
                com += pos;
            }
            double n = mol.getNumAtoms();
            com /= n;
            return com;
        }
    };

    struct electronCount {
        size_t total_electrons;
        size_t net_electrons;
        electronCount() : total_electrons( 0 ), net_electrons( 0 ) {}

        size_t count( const RDKit::ROMol& mol, const std::optional< adduct >& a ) const {
            size_t count = 0;
            for ( size_t i = 0; i < mol.getNumAtoms(); ++i ) {
                auto atom = mol.getAtomWithIdx( i );
                count += atom->getAtomicNum();
            }
            if ( a ) {
                auto atom = RDKit::Atom( a->symbol() );
                count += atom.getAtomicNum() - 1; // since adduct specified as neutral
            }
            return count;
        }

        bool validate( const RDKit::ROMol& mol, const std::optional< adduct >& a, int charge, int multiplicity ) {
            total_electrons = count( mol, a );
            net_electrons = count( mol, a ) - charge;
            return ((total_electrons + multiplicity) % 2 == 1);
        }
    };


    //////////////////////////
    template< package >
    struct printer {
        void operator()( const std::vector< std::string >& keywords
                         , const RDKit::ROMol& mol
                         , const std::pair<int,int>&
                         , std::optional< adduct > a
                         , std::ostream& os ) const {   }
    };

    template<> void
    printer<XYZ>::operator()(const std::vector< std::string >&
                             , const RDKit::ROMol& mol
                             , const std::pair<int,int>&
                             , std::optional< adduct > a
                             , std::ostream& os ) const {
        for ( size_t i = 0; i < mol.getNumAtoms(); ++i ) {
            auto atom = mol.getAtomWithIdx( i );
            auto pos = mol.getConformer().getAtomPos( i );
#if __GNUC__
            os << boost::format( "%1%\t%2%\t%3%\t%4%" ) % atom->getSymbol() % pos.x % pos.y % pos.z << std::endl;
#else
            os << std::format( "{}\t{:.7}\t{:.7}\t{:.7}", atom->getSymbol(), pos.x, pos.y, pos.z ) << std::endl;
#endif
        }
        if ( a ) {
            auto pos = a->pos( mol );
#if __GNUC__
            os << boost::format( "%1%\t%2%\t%3%\t%4%" ) % a->symbol() % pos.x % pos.y % pos.z << std::endl;
#else
            os << std::format( "{}\t{:.7}\t{:.7}\t{:.7}", a->symbol(), pos.x, pos.y, pos.z ) << std::endl;
#endif
        }
    }

    template<> void
    printer<MOPAC>::operator()(const std::vector< std::string >& keywords, const RDKit::ROMol& mol, const std::pair<int,int>& c
                               , std::optional< adduct > add
                               , std::ostream& os ) const {
        for ( const auto& keyword: keywords )
            os << keyword << "\t";
        if ( std::get<0>(c) != 0 ) {
            if ( std::find_if( keywords.begin(), keywords.end(), [](const auto& kw){
                return kw.find( "CHARGE" ) != std::string::npos || kw.find("charge") != std::string::npos; }) == keywords.end() ) {
#if __GNUC__
                os << boost::format( "CHARGE=%1%\t" ) % std::get<0>(c);
#else
                os << std::format( "CHARGE={}\t", std::get<0>(c) );
#endif
            }
            if ( std::get<1>(c) == 1 ) {
                if ( std::find_if( keywords.begin(), keywords.end(), [](const auto& kw){
                    return kw.find( "SINGLET" ) != std::string::npos || kw.find("singlet") != std::string::npos; }) == keywords.end() ) {
                    os << "SINGLET\t";
                }
            } else if ( std::get<1>(c) == 2 ) {
                if ( std::find_if( keywords.begin(), keywords.end(), [](const auto& kw){
                    return kw.find( "DOUBLET" ) != std::string::npos || kw.find("doublet") != std::string::npos; }) == keywords.end() ) {
                    os << "DOUBLET\t";
                }
            }
        }
        os << "\n\n\n";
        printer<XYZ>{}( keywords, mol, c, add, os );
    }

    template<> void
    printer<ORCA>::operator()(const std::vector< std::string >& keywords
                              , const RDKit::ROMol& mol
                              , const std::pair<int,int>& c
                              , std::optional< adduct > a
                              , std::ostream& os ) const {
        for ( const auto& keyword: keywords )
            os << keyword << std::endl;
        os << std::endl;
#if __GNUC__
        os << boost::format( "*\txyz\t%1%\t%2%" ) % std::get<0>(c) % std::get<1>(c) << std::endl;
#else
        os << std::format( "*\txyz\t{}\t{}", std::get<0>(c), std::get<1>(c) ) << std::endl;
#endif
        printer<XYZ>{}( keywords, mol, c, a, os );
        os << "*" << std::endl;
    }

    ///
    struct output_stream {
        mutable std::unique_ptr< std::ofstream > _;
        std::ostream& operator()( const std::string& name ) const {
            if ( name == "-" )
                return std::cout;
            else {
                _ = std::make_unique< std::ofstream >( name );
                return *_;
            }
        }
    };

}

int
main(int argc, char *argv[])
{
    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "smiles",       po::value< std::vector< std::string > >(),  "smiles" )
            ( "output,o",     po::value< std::string >()->default_value("-"), "output file, '-' for stdout" )
            ( "keywords,k",   po::value< std::vector< std::string > >()->multitoken(), "keywords" )
            ( "input,i",      po::value< std::string >()->default_value("mopac"), "input format, mopac|orca|xyz" )
            ( "charge,c",     po::value< int >()->default_value( 0 ),   "charge" )
            ( "multiplicity,m", po::value< int >()->default_value( 1 ), "multiplicity(2S+1)" )
            ( "adduct,a", po::value< std::string >(), "adduct such as H, Na, K" )
            ( "nearby,n", po::value< std::string >()->default_value( "O" ), "where to place adduct" )
            ( "distance,d", po::value< double >()->default_value( 2.3 ), "Bond distance in angstrome" )
            ;
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }

    std::vector< std::shared_ptr< RDKit::ROMol > > mols;

    if ( vm.count( "help" ) || (vm.count( "smiles" ) == 0) ) {
        std::cout << description;
        return 0;
    }

    const auto& keywords = vm[ "keywords" ].as< std::vector< std::string > >();

    if ( vm.count("smiles") ) {

        using _P = std::shared_ptr< RDKit::ROMol >;

        output_stream out;

        for ( auto arg: vm[ "smiles" ].as< std::vector< std::string > >() ) {
            if ( auto mol = _P( RDKit::SmilesToMol( arg ) ) ) {
                if ( auto mol_h = _P( RDKit::MolOps::addHs( *mol ) ) ) {
                    RDKit::DGeomHelpers::EmbedMolecule( *mol_h );
                    RDKit::UFF::UFFOptimizeMolecule( *mol_h );
                    std::pair< int, int > charge{ vm[ "charge" ].as<int>(), vm[ "multiplicity" ].as<int>() };

                    std::optional< adduct > a;
                    if ( vm.count( "adduct" ) ) {
                        a = adduct{ vm["adduct"].as<std::string>(), vm["nearby"].as<std::string>(), vm["distance"].as<double>() };
                    }

                    if ( vm[ "input" ].as<std::string>() == "xyz" ) {
                        printer<XYZ>{}(keywords, *mol_h, charge, a, out( vm[ "output" ].as< std::string >() ) );
                    } else if ( vm[ "input" ].as<std::string>() == "orca" ) {
                        printer<ORCA>{}(keywords, *mol_h, charge, a, out( vm[ "output" ].as< std::string >() ) );
                    } else if ( vm[ "input" ].as<std::string>() == "mopac" ) {
                        printer<MOPAC>{}(keywords, *mol_h, charge, a, out( vm[ "output" ].as< std::string >() ) );
                    } else {
                        printer<XYZ>{}(keywords, *mol_h, charge, a, out( vm[ "output" ].as< std::string >() ) );
                    }

                    electronCount ec;
                    if ( not ec.validate( *mol, a, std::get<0>(charge), std::get<1>(charge) ) ) {
                        int multiplicity = std::get<1>(charge);
                        std::cerr << "output: " << vm["output"].as< std::string >();
                        std::cerr << "\tCheck electron/spin multiplicity configuration: total_eectrons " << ec.total_electrons
                                  << ", multiplicity = " << multiplicity
                                  << ", (ec.total_electrons - multiplicity) = " << (ec.total_electrons - multiplicity)
                                  << std::endl;
                    }
                }
            }
        }
    }
}
