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

#include "accession.hpp"

namespace mzml {

    class accession;
    typedef std::variant< const float *, const double * > data_ptr;

    class binaryDataArray {
        size_t encodedLength_;
        std::string decoded_;
        accession ac_;
    public:
        binaryDataArray( size_t length = 0
                         , mzml::accession&& ac = {}
                         , std::string&& decoded = {} );
        binaryDataArray( const binaryDataArray& t );


        operator bool () const;
        const accession& accession() const;
        size_t size() const;
        size_t length() const;
        data_ptr data() const;

        static binaryDataArray make_instance( const pugi::xml_node& node );
    };
} // namespace
