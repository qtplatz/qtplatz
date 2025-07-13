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

#include "serializer.hpp"
#include "mzmlspectrum.hpp"
#include "mzmlreader.hpp"
#include <adportable/debug.hpp>
#include <pugixml.hpp>
#include <variant>

using namespace mzml;

namespace {
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}

namespace mzml {
    std::shared_ptr< mzMLSpectrum >
    serializer::deserialize( const char * data, size_t )
    {
        pugi::xml_document doc;
        if ( doc.load_string( data ) ) {
            if ( auto node = doc.select_node( "spectrum" ) ) {
                auto v = mzMLReader{}(node.node() );
                return std::visit( overloaded {
                        []( std::shared_ptr< mzMLChromatogram> sp )->std::shared_ptr< mzMLSpectrum >{ return nullptr; }
                            , []( std::shared_ptr< mzMLSpectrum > t ) { return t; }
                            }
                    , v );
            }
        }
        return nullptr;
    }
}
