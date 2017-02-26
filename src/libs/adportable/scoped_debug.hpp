// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include <sstream>
#include <chrono>
#include "debug.hpp"

namespace adportable {

    template< class T = debug > class scoped_debug : public T {
        std::chrono::steady_clock::time_point trig_point_;
    public:
        scoped_debug(const char * file = 0
                     , const int line = 0) : T( file, line )
                                           , trig_point_( std::chrono::steady_clock::now() ) {
        }
        ~scoped_debug(void) {
            auto duration = std::chrono::steady_clock::now() - trig_point_;
            static_cast< T& >(*this)
                << "\t"
                << double(std::chrono::duration_cast< std::chrono::microseconds >( duration ).count()) * 1e-6
                << "s.";
        }
    };
}

#define ScopedDebug(t) adportable::scoped_debug<adportable::debug> t(__FILE__, __LINE__)
