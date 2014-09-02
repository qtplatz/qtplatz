// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "pugixml.hpp"
#include <adportable/xml_serializer.hpp>
#include <string>
#include <sstream>

namespace pugi {

    class xmlhelper {
        xml_document dom;
        xmlhelper( const xmlhelper& ) = delete;
    public:
        xmlhelper() {}

        template<class T> xmlhelper( const T& data ) {
            (*this)(data);
        }

        template<class T> bool operator()( const T& data ) {
            std::wstringstream o;
            if ( adportable::xml::serialize<T>()(data, o) ) {
                auto status = dom.load( o );
                if ( status.status == pugi::status_ok || status.status == pugi::status_end_element_mismatch )
                    return true;
            }
            return false;
        }
        xml_document& doc() { return dom; }
    };

}

