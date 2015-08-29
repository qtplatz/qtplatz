/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "adicontroller_global.hpp"
#include <cstdint>
#include <vector>

namespace adicontroller {
    
    class ADICONTROLLERSHARED_EXPORT octet_array {
        octet_array( const octet_array& t ) = delete;
        octet_array& operator = ( const octet_array& ) = delete;
#if defined _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif
        std::vector< uint8_t > d_;
#if defined _MSC_VER
# pragma warning(pop)
#endif
    public:
        octet_array() {}
        octet_array( size_t n );
        ~octet_array();
        const uint8_t * data() const;
        uint8_t * data();
        size_t size() const;
        void resize( size_t n );
    };

}
