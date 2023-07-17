// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include <regex>
#include <iostream>

int
main()
{
    std::regex ex( "(MSLock)" );
    std::vector< std::string > texts = { "acquite.title"
                                         , "create"
                                         , "MSLock"
    };

    for ( const auto& key: texts ) {
        std::match_results< typename std::basic_string< char >::const_iterator > match;
        if ( std::regex_search( key, match, ex ) ) {
            std::cout << "-- match -- " << key << std::endl;
        } else {
            std::cout << "-- not match -- " << key << std::endl;
        }
    }

}
