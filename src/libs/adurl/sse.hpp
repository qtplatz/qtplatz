// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "adurl_global.h"
#include <functional>

// SSE client for socfpga/dg-httpd server

namespace adurl {

    class ADURLSHARED_EXPORT sse {
    public:

        class ADURLSHARED_EXPORT request_timeout : public  std::exception {};
        class ADURLSHARED_EXPORT error_reply : public std::exception {};

        ~sse();
        sse( const char * server /* = "dg-httpd"*/, const char * path /* = "/dg/ctl?events" */, const char * port = "80" );
        
        void exec( std::function< void( const char * /* event */, const char * /* data */ ) > callback );
        void stop();

    private:
        class impl;
        impl * impl_;
    };
    
}

