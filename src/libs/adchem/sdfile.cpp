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

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
# pragma warning(disable:4503)
# pragma warning(disable:4267) // convesrion from size_t to unsigned int
#endif

#include "sdfile.hpp"
#include "sdfile_parser.hpp"
#include <adportable/debug.hpp>

#include <RDGeneral/Invariant.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <RDGeneral/RDLog.h>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/map.hpp>

#include <fstream>

using namespace adchem;

SDFile::SDFile( const std::string& filename, bool sanitize, bool removeHs, bool strictParsing )
    : molSupplier_( std::make_shared< RDKit::SDMolSupplier >( filename, sanitize, removeHs, strictParsing ) )
    , filename_( filename )
{
}

SDFile::iterator
SDFile::begin()
{
    return sdfile_iterator( *molSupplier_, 0 );
}

SDFile::const_iterator
SDFile::begin() const
{
    return sdfile_iterator( *molSupplier_, 0 );
}

SDFile::iterator
SDFile::end()
{
    return sdfile_iterator( *molSupplier_, size() );
}

SDFile::const_iterator
SDFile::end() const
{
    return sdfile_iterator( *molSupplier_, size() );
}

SDFile::size_type
SDFile::size() const
{
    return molSupplier_->length();
}

// static
bool
SDFile::parseItemText( const std::string& text, std::map< std::string, std::string >& data )
{
    std::string::size_type pos = text.find_first_of( ">" );

    if ( pos != std::string::npos ) {

        std::string xstr;

        for ( std::string::const_iterator it = text.begin() + pos; it < text.end(); ++it ) {
            // if ( std::isprint( unsigned(*it) ) ) // formula contains 128> char that cause an assersion error on VS2012
            xstr += *it;
        }
        
        // std::ofstream of( "text.txt" );
        // of << xstr;

        sdfile_parser< std::string::const_iterator > parser;
        nodes_type nodes;
        
        std::string::const_iterator it = xstr.begin();
        std::string::const_iterator end = xstr.end();

        if ( boost::spirit::qi::parse( it, end, parser, nodes ) ) {
            for ( const auto& node: nodes )
                data[ node.first ] = node.second;
        } else {
            adportable::debug(__FILE__, __LINE__) << "associatedData parse failed";
        }

    }
	return true;
}

sdfile_iterator::sdfile_iterator( RDKit::SDMolSupplier& supplier
                                  , size_t idx ) : supplier_( supplier )
                                                 , idx_( idx )
{
}

sdfile_iterator::sdfile_iterator( const sdfile_iterator& t ) : supplier_( t.supplier_ )
                                                             , idx_( t.idx_ )
{
}

const sdfile_iterator&
sdfile_iterator::operator ++ ()
{
    ++idx_;
    return *this;
}

sdfile_iterator
sdfile_iterator::operator + ( int distance ) const
{
    return sdfile_iterator( supplier_, idx_ + distance );
}

bool
sdfile_iterator::operator != ( const sdfile_iterator& rhs ) const
{
    return idx_ != rhs.idx_;
}

sdfile_iterator::operator RDKit::ROMol * () const
{
    const_cast< sdfile_iterator *>(this)->fetch();
    return mol_.get();
}

std::string
sdfile_iterator::itemText() const
{
    return supplier_.getItemText( idx_ );
}

bool
sdfile_iterator::fetch()
{
    if ( idx_ < size_t( supplier_.length() ) ) {
        mol_.reset( supplier_[ idx_ ] );
        return true;
    }
    mol_.reset();
    return false;
}
