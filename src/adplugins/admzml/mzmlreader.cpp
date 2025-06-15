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

namespace mzml {

    template<>
    std::pair< const pugi::xml_node, const pugi::xml_node >
    mzMLReader< dataTypeSpectrum >::getArrayNodes( const pugi::xml_node node ) const {
        if (auto intens= node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000515']]")) {
            if (auto mz= node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000514']]")) {
                return { mz.node(), intens.node() };
            }
        }
        return {};
    }

    template<>
    std::pair< const pugi::xml_node, const pugi::xml_node >
    mzMLReader< dataTypeChromatogram >::getArrayNodes( const pugi::xml_node node ) const {

        if (auto intens= node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000515']]")) {
            if (auto time = node.select_node("binaryDataArrayList/binaryDataArray[cvParam[@accession='MS:1000595']]")) {
                return { time.node(), intens.node()};
            }
        }
        return {};
    }

} // namespace
