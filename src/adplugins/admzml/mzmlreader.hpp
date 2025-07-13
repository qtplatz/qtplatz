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

#include "mzmlchromatogram.hpp"
#include "mzmlspectrum.hpp"
#include <pugixml.hpp>
#include <memory>

namespace mzml {

    using datum_variant_t = std::variant<
        std::shared_ptr< mzml::mzMLSpectrum >
        , std::shared_ptr< mzml::mzMLChromatogram >
        >;

    class mzMLReader {
    public:
        ~mzMLReader();
        mzMLReader();
        datum_variant_t operator()( const pugi::xml_node& node ) const;
    private:
        std::pair< const pugi::xml_node, const pugi::xml_node > getSpectrumArrays( const pugi::xml_node& ) const;
        std::pair< const pugi::xml_node, const pugi::xml_node > getChromatogramArrays( const pugi::xml_node& ) const;
        std::pair< const pugi::xml_node, const pugi::xml_node > getArrays( const pugi::xml_node& ) const;
    };

} // namespace
