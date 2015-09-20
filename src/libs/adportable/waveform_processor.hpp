// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015 MS-Cheminformatics LLC
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

#include <vector>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace adportable {

    class waveform_processor {
    public:
        template<class Iterator, class T>
        Iterator find_threshold_element( Iterator beg, Iterator end, T level, bool& flag ) {
            if ( beg != end ) {
                flag = *beg < level; // positive-peak-front if true
                while ( ++beg != end ) {
                    if ( ( *beg < level ) != flag )
                        return beg;
                }
            }
            return end;
        }
    };

}

