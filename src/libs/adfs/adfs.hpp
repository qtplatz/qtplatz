// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include "adfs_global.h"
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adfs/attributes.hpp>
#include <exception>
#include <compiler/pragma_warning.hpp>

namespace adfs {

    class /* ADFSSHARED_EXPORT */ exception : public std::exception {
    public:
        exception( const std::string& msg, const char * cat ) : message_(msg), category_(cat) {}

        const std::string& message() const { return message_; }
        const std::string& category() const { return category_; }

    private:
        pragma_msvc_warning_push_disable_4251
        std::string message_;
        std::string category_;
        pragma_msvc_warning_pop
    };

    ADFSSHARED_EXPORT const char * null_safe( const char * s );

    ADFSSHARED_EXPORT std::wstring create_uuid();
}


