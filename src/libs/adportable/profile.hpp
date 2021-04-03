/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
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

#ifndef PROFILE_HPP
#define PROFILE_HPP

#include "adportable_global.h"
#include <string>

namespace adportable {

    class ADPORTABLESHARED_EXPORT profile {
    public:
        profile();
        template< class char_type > static std::basic_string<char_type> user_login_name();
        template< class char_type > static std::basic_string<char_type> user_login_id();
        template< class char_type > static std::basic_string<char_type> user_data_dir();
        template< class char_type > static std::basic_string<char_type> computer_name();

        // ~/.config on macOS and Linux; ~/AppData/Roaming on Windows
        template< class char_type > static std::basic_string<char_type> user_config_dir();

        // ~/.config on macOS and Linux; ~/AppData/Local on Windows
        template< class char_type > static std::basic_string<char_type> user_local_config_dir();
    };
#if defined WIN32
    template<> ADPORTABLESHARED_EXPORT std::string profile::user_login_name<char>();
    template<> ADPORTABLESHARED_EXPORT std::string profile::user_login_id<char>();
    template<> ADPORTABLESHARED_EXPORT std::string profile::user_data_dir<char>();
    template<> ADPORTABLESHARED_EXPORT std::string profile::computer_name<char>();
    template<> ADPORTABLESHARED_EXPORT std::string profile::user_config_dir<char>();
    template<> ADPORTABLESHARED_EXPORT std::wstring profile::user_login_name<wchar_t>();
    template<> ADPORTABLESHARED_EXPORT std::wstring profile::user_login_id<wchar_t>();
    template<> ADPORTABLESHARED_EXPORT std::wstring profile::user_data_dir<wchar_t>();
    template<> ADPORTABLESHARED_EXPORT std::wstring profile::computer_name<wchar_t>();
    template<> ADPORTABLESHARED_EXPORT std::wstring profile::user_config_dir<wchar_t>();

    template<> ADPORTABLESHARED_EXPORT std::string profile::user_local_config_dir<char>();
    template<> ADPORTABLESHARED_EXPORT std::wstring profile::user_local_config_dir<wchar_t>();
#endif
}

#endif // PROFILE_HPP
