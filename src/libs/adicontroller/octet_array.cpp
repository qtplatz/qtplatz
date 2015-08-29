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

#include "octet_array.hpp"
#include <vector>

namespace adicontroller {

    octet_array::octet_array( size_t n ) : d_( n )
    {
    }

    octet_array::~octet_array()
    {
    }

    const uint8_t *
    octet_array::data() const
    {
        return d_.data();
    }

    uint8_t *
    octet_array::data()
    {
        return d_.data();
    }

    size_t
    octet_array::size() const
    {
        return d_.size();
    }

    void
    octet_array::resize( size_t n )
    {
        return d_.resize( n );
    }

}
