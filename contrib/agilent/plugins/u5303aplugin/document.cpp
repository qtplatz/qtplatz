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

#include "document.hpp"
#include <memory>
#include <u5303a/digitizer.hpp>

using namespace u5303a;

document * document::instance_ = 0;
std::mutex document::mutex_;

namespace u5303a { namespace detail {
        struct remover {
            ~remover() {
                if ( document::instance_ ) {
                    std::lock_guard< std::mutex > lock( document::mutex_ );
                    if ( document::instance_ )
                        delete document::instance_;
                }
            };
            static remover _remover;
        };
    }
}
    
document::document() : digitizer_( new u5303a::digitizer )
{
}

document::~document()
{
    delete digitizer_;
}

document *
document::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new document;
    }
    return instance_;
}

bool
document::u5303a_connect()
{
	return digitizer_->peripheral_initialize();
}
