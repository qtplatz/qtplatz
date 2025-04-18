/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheinformatics LLC, Toin, Mie Japan
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
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/GeneralFileReader.h>
#include <GraphMol/FileParsers/MolWriters.h>
#include <GraphMol/GraphMol.h>
#include <GraphMol/MolTransforms/MolTransforms.h>
#include <GraphMol/ShapeHelpers/ShapeUtils.h>
#include <GraphMol/ShapeHelpers/ShapeEncoder.h>
#include <GraphMol/ROMol.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/ForceFieldHelpers/UFF/UFF.h>
#include <Geometry/UniformGrid3D.h>
#include <boost/program_options.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <ratio>

namespace po = boost::program_options;

namespace {

    template < typename T >
    class IndexIterable {
    public:
        explicit IndexIterable( const T& container ) : container_( container ) {}
        auto begin() const { return Iterator( container_, 0 ); }
        auto end() const { return Iterator( container_, container_.size() ); }
    private:
        struct Iterator {
            using iterator_category = std::random_access_iterator_tag;
            using value_type = int;
            using difference_type = std::ptrdiff_t;
            using pointer = const int*;
            using reference = int;

            const T& container;
            size_t index;
            Iterator( const T& c, size_t i ) : container( c ), index(i) {}
            int operator * () const { return container.getVal(index); }
            Iterator& operator++() { ++index; return *this; }
            bool operator != (const Iterator& other ) const { return index != other.index; }
        };
        const T& container_;
    };


    struct print_molvolume {
        print_molvolume() {}
        void operator()( const RDKit::ROMol& mol, std::ostream& os ) const {

            auto conf = mol.getConformer();
            MolTransforms::canonicalizeConformer(conf);
            RDGeom::Point3D leftBottom, rightTop;
            RDKit::MolShapes::computeConfBox(conf, leftBottom, rightTop);
            const double boxMargin = 2.0;
            const double gridSpacing = 0.2;
            auto sideLen = std::make_tuple(rightTop.x - leftBottom.x + 2 * boxMargin
                                           , rightTop.y - leftBottom.y + 2 * boxMargin
                                           , rightTop.z - leftBottom.z + 2 * boxMargin );

            RDGeom::UniformGrid3D shape(std::get<0>(sideLen), std::get<1>(sideLen), std::get<2>(sideLen), gridSpacing);
            RDKit::MolShapes::EncodeShape( mol, shape, -1 );
            auto voxelVol = gridSpacing*gridSpacing*gridSpacing;
            if ( auto occVect = shape.getOccupancyVect() ) {
                auto counts = std::count_if( IndexIterable( *occVect ).begin()
                                             , IndexIterable( *occVect ).end()
                                             , [](const auto v){ return v == 3; });

                os << std::format( "occVect->size = {}, molVolume={}", counts, voxelVol * counts ) << std::endl;
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
            ;

        // po::positional_options_description p;
        // p.add( "args",  -1 );
        //po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::store( po::command_line_parser( argc, argv ).options( description ).run(), vm );
        po::notify(vm);
    }

    std::vector< std::shared_ptr< RDKit::ROMol > > mols;

    if ( vm.count( "help" ) || (vm.count( "smiles" ) == 0) ) {
        std::cout << description;
        return 0;
    }

    if ( vm.count("smiles") ) {

        using _P = std::shared_ptr< RDKit::ROMol >;

        for ( auto arg: vm[ "smiles" ].as< std::vector< std::string > >() ) {
            if ( auto mol = _P( RDKit::SmilesToMol( arg ) ) ) {
                if ( auto mol_h = _P( RDKit::MolOps::addHs( *mol ) ) ) {
                    RDKit::DGeomHelpers::EmbedMolecule( *mol_h );
                    RDKit::UFF::UFFOptimizeMolecule( *mol_h );
                    if ( vm[ "output" ].as< std::string >() == "-" )
                        print_molvolume()(*mol_h, std::cout );
                    else {
                        std::ofstream ofs( vm[ "output" ].as<std::string>() );
                        print_molvolume()(*mol_h, ofs );
                    }
                }
            }
        }
    }
}
