/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "adcontrols_global.h"
#include <cstdlib>

namespace adcontrols {

    namespace toe {

        // iterator for fast access to isotopes in table-of-element
        struct isotope { double mass; double abundance; };

        class isotopes {
            typedef const isotope * const_iterator;
            typedef size_t size_type;
        public:
            isotopes( const isotope * p, size_t size ) : p_(p), size_(size) {}
            inline const_iterator begin() const { return p_; }
            inline const_iterator end() const { return p_ + size_; }
            inline size_type size() const { return size_; }
            inline const isotope& operator [] ( int idx ) const { return p_[ idx ]; }
        private:
            const isotope * p_;
            const size_t size_;
        };
    }

}
