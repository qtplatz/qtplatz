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

#pragma once
#include "mzmlspectrum.hpp"
#include "mzmlchromatogram.hpp"
#include "binarydataarray.hpp"
#include "mzmlspectrum.hpp"
#include "mzmlchromatogram.hpp"

#include <pugixml.hpp>
#include <type_traits>
#include <memory>

namespace mzml {

    enum dataType { dataTypeSpectrum, dataTypeChromatogram };

    using datum_variant_t = std::variant< std::shared_ptr< mzMLSpectrum >
                                          , std::shared_ptr< mzMLChromatogram > >;


    template< dataType T >
    struct mzMLReader {

        using mzMLDatumType = std::conditional< T == dataTypeSpectrum, mzMLSpectrum, mzMLChromatogram >::type;

        mzMLReader() {}

        std::pair< const pugi::xml_node, const pugi::xml_node > getArrayNodes( const pugi::xml_node ) const;

        datum_variant_t
        operator()( const pugi::xml_node& node ) const {
            size_t count = node.select_node( "binaryDataArrayList/@count" ).attribute().as_uint();
            if ( count == 2 ) {
                auto nodes = getArrayNodes( node );
                auto prime = binaryDataArray::make_instance( std::get<0>( nodes ) );
                auto secondi = binaryDataArray::make_instance( std::get<1>( nodes ) );
                if ( prime.length() == secondi.length()) {
                    return std::make_shared< typename mzMLReader<T>::mzMLDatumType  >( prime, secondi, node );
                }
            }
            return {};
        }
    };

    template<>
    std::pair< const pugi::xml_node, const pugi::xml_node >
    mzMLReader< dataTypeSpectrum >::getArrayNodes( const pugi::xml_node node ) const;

    template<>
    std::pair< const pugi::xml_node, const pugi::xml_node >
    mzMLReader< dataTypeChromatogram >::getArrayNodes( const pugi::xml_node node ) const;

} // namespace
