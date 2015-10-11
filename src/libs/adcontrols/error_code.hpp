/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "adcontrols_global.h"
#include <string>
#include <system_error>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT error_code {
    public:
        enum category { noerror, std_error, adcontrols_error };
        
        error_code();
        error_code( const error_code& );

        void assign( int ec, const std::string& message );
        void assign( std::error_code& ec );

        std::string message() const;

    private:
        category cat_;
        int code_;

#if defined _MSC_VER
# pragma warning( push )
# pragma warning( disable:4251 )
#endif
        std::string message_;
        std::error_code ec_;

#if defined _MSC_VER
# pragma warning( pop )
#endif        

    };

}



