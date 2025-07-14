// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "mzmlreader.hpp"
#include "mzmldatumbase.hpp"
#include "mzmlspectrum.hpp"
#include <adportable/debug.hpp>

namespace mzml {

    mzMLReader::mzMLReader()
    {
    }

    mzMLReader::~mzMLReader()
    {
    }

    std::pair< const pugi::xml_node, const pugi::xml_node >
    mzMLReader::getSpectrumArrays( const pugi::xml_node& node ) const
    {
        if (auto intens= node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000515']]")) {
            if (auto mz= node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000514']]")) {
                return { mz.node(), intens.node() };
            }
        }
        return {};
    }

    std::pair< const pugi::xml_node, const pugi::xml_node >
    mzMLReader::getChromatogramArrays( const pugi::xml_node& node ) const
    {
        if (auto intens= node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000515']]")) {
            if (auto time = node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000595']]")) {
                return { time.node(), intens.node()};
            }
        }
        return {};
    }

    std::pair< const pugi::xml_node, const pugi::xml_node >
    mzMLReader::getArrays( const pugi::xml_node& node ) const
    {
        if ( node.name() == std::string( "spectrum" ) )
            return getSpectrumArrays( node );
        if ( node.name() == std::string( "chromatogram" ) )
            return getChromatogramArrays( node );
        return {};
    }

    datum_variant_t
    mzMLReader::operator()( const pugi::xml_node& node ) const
    {
        size_t count = node.select_node( "binaryDataArrayList/@count" ).attribute().as_uint();
        if ( count == 2 ) {
            auto arrays = getArrays( node );
            auto prime = binaryDataArray::make_instance( std::get<0>( arrays ) );
            auto secondi = binaryDataArray::make_instance( std::get<1>( arrays ) );
            if ( prime.length() == secondi.length()) {
                if ( node.name() == std::string( "spectrum")  ) {
                    return  std::make_shared< mzMLSpectrum >( prime, secondi, node );
                } else if ( node.name() == std::string( "chromatogram" ) ) {
                    return  std::make_shared< mzMLChromatogram >( prime, secondi, node );
                }
            }
        }
        return {};
    }


} // namespace
