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

#include <QString>
#include <string>
#include "qtwrapper_global.h"

namespace qtwrapper {

    struct QTWRAPPERSHARED_EXPORT qstring {
        QString impl_;
        qstring() {}
        qstring( const std::wstring& );
        inline operator QString& () { return impl_; }
        static QString copy( const std::wstring& );
    };

    struct QTWRAPPERSHARED_EXPORT wstring {
        std::wstring impl_;
        wstring() {}
        wstring( const QString& );
        inline operator std::wstring& () { return impl_; }
        inline const wchar_t * c_str() const { return impl_.c_str(); }
        static std::wstring copy( const QString& );
    };

}

