// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <QString>
#include <string>

namespace qtwrapper {

    struct qstring {
        QString impl_;
        qstring() {}
        qstring( const std::wstring& );
        inline operator QString& () { return impl_; }
        static QString copy( const std::wstring& );
    };

    struct wstring {
        std::wstring impl_;
        wstring() {}
        wstring( const QString& );
        inline operator std::wstring& () { return impl_; }
        static std::wstring copy( const QString& );
    };

}

