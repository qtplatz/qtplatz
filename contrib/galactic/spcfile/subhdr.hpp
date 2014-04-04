/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#ifndef SUBHDR_HPP
#define SUBHDR_HPP

#pragma once
#include "spc_h.hpp"
#include "spcfile_global.hpp"
#include <ostream>

namespace galactic {

    class SPCFILESHARED_EXPORT subhdr {
    public:
        subhdr( const SUBHDR * p, const SPCHDR * );
        subhdr( const subhdr& );

        uint8_t subflgs() const;
        int8_t  subexp() const;
        int16_t subindx() const;
        float subtime() const;
        float subnext() const;
        float subnois() const;
        uint32_t subnpts() const;
        uint32_t subscan() const;
        float subwlevel() const;
        double operator [] ( size_t idx ) const;
        void dump_subhdr( std::ostream& ) const;

    private:
        const SUBHDR * p_;
        const SPCHDR * spchdr_;
        int fexp_;
    };

}

#endif // SUBHDR_HPP
