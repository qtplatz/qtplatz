/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
# pragma warning(disable:4503)
# pragma warning(disable:4267) // convesrion from size_t to unsigned int
#endif

#include <adcontrols/chemicalformula.hpp>
#include "drawing.hpp"
#include "sdfile.hpp"
#include "sdfile_parser.hpp"
#include <adportable/debug.hpp>

#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/inchi.h>
#include <RDGeneral/Invariant.h>
#include <RDGeneral/RDLog.h>

#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/map.hpp>
#include <atomic>
#include <codecvt>
#include <execution>
#include <fstream>
#include <locale>
#include <mutex>

using namespace adchem;

SDFile::SDFile( const std::string& filename, bool sanitize, bool removeHs, bool strictParsing )
    : molSupplier_( std::make_unique< RDKit::SDMolSupplier >( filename, sanitize, removeHs, strictParsing ) )
    , filename_( filename )
{
}

size_t
SDFile::size() const
{
    return molSupplier_->length();
}

SDFile::operator bool() const
{
    return molSupplier_ != nullptr;
}

RDKit::SDMolSupplier&
SDFile::molSupplier()
{
    return *molSupplier_;
}

adchem::SDMol
SDFile::at( size_t index )
{
    return SDMol( this, index );
}

std::vector< SDMol >
SDFile::populate( std::function< void(size_t) > progress )
{
    std::vector< size_t > indices( this->size() );
    std::iota( indices.begin(), indices.end(), 0 );
    std::vector< SDMol > d;
    d.reserve( size() );
    // std::for_each( std::execution::par
    std::for_each( indices.begin(), indices.end(), [&]( auto idx ){ d.emplace_back( at( idx ) ); progress( d.size() ); });
    return d;
}

std::vector< std::pair< std::string, std::string > >
SDFile::parseItemText( const std::string& text )
{
    std::vector< std::pair< std::string, std::string > > data;

    std::string::size_type pos = text.find_first_of( ">" );
    if ( pos != std::string::npos ) {

        // auto xstr = text.substr( pos ); //cvt.from_bytes( text.substr( pos ) );
        sdfile_parser< std::string::const_iterator > parser;
        nodes_type nodes;

        std::string::const_iterator it = text.begin() + pos;
        std::string::const_iterator end = text.end();

        if ( boost::spirit::qi::parse( it, end, parser, nodes ) ) {
            for ( auto& node: nodes ) {
                boost::trim( node.second );
                data.emplace_back( node );
            }
        } else {
            adportable::debug(__FILE__, __LINE__) << "associatedData parse failed";
        }
    }
	return data;
}

// std::vector< SDFileData >
// SDFile::toData( std::function< bool(size_t) > progress )
// {
//     std::vector< SDFileData > d;
//     std::vector< size_t > indices( this->size() );
//     std::iota( indices.begin(), indices.end(), 0 );
// #if __APPLE__  || ! HAVE_TBB // Xcode clang-13 does not support execution::parallel
//     std::for_each( indices.begin()
//                    , indices.end(), [&]( auto idx ){
//                        d.emplace_back( SDFileData( sdfile_iterator( *molSupplier_, idx ) ) );
//                        progress( d.size() );
//                    });
// #else
//     std::mutex mutex;
//     std::for_each( std::execution::par
//                    , indices.begin()
//                    , indices.end(), [&]( auto idx ){
//                        std::lock_guard<std::mutex> guard(mutex);
//                        d.emplace_back( SDFileData( sdfile_iterator( *molSupplier_, idx ) ) );
//                        progress( d.size() );
//                    });
// #endif
//     return {};
// }

///////////////////////////////////////
