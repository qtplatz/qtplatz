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
    std::string rex1 = R"((.*)://([^/]*)(/.*)$)";
    std::vector< std::string > texts = {
        "https://api.jstage.jst.go.jp/searchapi/do?service=3&material=Mass%20Spectrometry"
    };
    std::regex ex( rex1, std::regex::extended );

    std::cout << "regex: " << rex1 << std::endl;
    for ( const auto& key: texts ) {
        std::match_results< typename std::basic_string< char >::const_iterator > match;
        if ( std::regex_match( key, match, ex ) ) {
            std::cout << "-- match -- " << key << std::endl;
            for ( size_t i = 0; i < match.size(); ++i )
                std::cout << "\tmatch." << i << "\t" << match[i].str() << std::endl;
        } else {
            std::cout << "-- not match -- " << key << std::endl;
        }
    }

#if 0
    std::string rex1 = R"(^.*\(SPD-40_A.*$)";
    std::string rex2 = R"(^SFE([0-9]+).*SFC)";
    //std::regex ex( "(MSLock)" );
    std::regex ex( rex1, std::regex::extended );
    std::regex ex2( rex2, std::regex::extended );
    std::vector< std::string > texts = { "acquite.title"
                                         , "create"
                                         , "MSLock"
                                         , "SFE10sSFC_C8694PSS_PGE2+VK1+ARA_26+480+210nM_DC-400V94uA_0002uL_0001_SPD-40_A-Ch1_"
                                         , "SFE8sSFC_C8694PSS_PGE2+VK1+ARA_26+480+210nM_DC-400V94uA_0002uL_0001(SPD-40_A-Ch1)"
                                         , "SFE9sSFC_C8694PSS_PGE2+VK1+ARA_26+480+210nM_DC-400V94uA_0002uL_0001(SPD-40_A-Ch1)"
                                         , "SFE10sSFC_C8694PSS_PGE2+VK1+ARA_26+480+210nM_DC-400V94uA_0002uL_0001(SPD-40_A-Ch1)"
                                         , "SFE12sSFC_C8694PSS_PGE2+VK1+ARA_26+480+210nM_DC-400V94uA_0002uL_0001(SPD-40_A-Ch1)"
                                         , "SFE12SFC_C8694PSS_PGE2+VK1+ARA_26+480+210nM_DC-400V94uA_0002uL_0001(SPD-40_A-Ch1)"
    };

    std::cout << "regex: " << rex1 << std::endl;
    for ( const auto& key: texts ) {
        std::match_results< typename std::basic_string< char >::const_iterator > match;
        if ( std::regex_match( key, match, ex ) ) {
            std::cout << "-- match -- " << key << std::endl;
            for ( size_t i = 0; i < match.size(); ++i )
                std::cout << "\tmatch." << i << "\t" << match[i].str() << std::endl;
        } else {
            std::cout << "-- not match -- " << key << std::endl;
        }
    }

    std::cout << "\nregex: " << rex2 << std::endl;
    for ( const auto& key: texts ) {
        std::match_results< typename std::basic_string< char >::const_iterator > match;
        if ( std::regex_search( key, match, ex2 ) ) {
            std::cout << "-- match -- " << key << std::endl;
            for ( size_t i = 0; i < match.size(); ++i )
                std::cout << "\tmatch." << i << "\t" << match[i].str() << std::endl;
        } else {
            std::cout << "-- not match -- " << key << std::endl;
        }
    }
#endif
}
