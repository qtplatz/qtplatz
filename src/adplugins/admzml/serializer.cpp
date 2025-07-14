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
#include <adportable/debug.hpp>
#include <adportable/bzip2.hpp>
#include <pugixml.hpp>


using namespace mzml;

namespace {
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}

namespace mzml {

    std::shared_ptr< pugi::xml_document >
    serializer::deserialize( const char * data, size_t length )
    {
        if ( adportable::bzip2::is_a( data, length ) ) {
            std::string inflated;
            adportable::bzip2::decompress( inflated, data, length );
            return deserialize( inflated.data(), inflated.size() );
        }

        auto doc = std::make_shared< pugi::xml_document >();
        if ( doc->load_string( data ) ) {
            return doc;
        }
        return nullptr;
    }
}
