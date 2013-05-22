/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include <adportable/timer.hpp>
#include <iostream>

namespace adportable {

    template<class stream = std::ostream>
    class scope_timer : public adportable::timer {
        const char * text_;
        stream& ostrm;
    public:
        scope_timer( const char * text, stream& o ) : text_( text ), ostrm( o ) {
        }
        scope_timer( const char * text ) : text_( text ), ostrm( std::cout ) {
        }
        ~scope_timer() {
            ostrm << text_ << elapsed() << " us" << std::endl;
        }
    };

}
