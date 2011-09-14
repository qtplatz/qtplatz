/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef CONTROLMETHODHELPER_HPP
#define CONTROLMETHODHELPER_HPP

#include "controlmethodC.h"

namespace adinterface {

    class ControlMethodInstInfo {
        ControlMethod::InstInfo info_;
    public:
        ControlMethodInstInfo();
        ControlMethodInstInfo( const ControlMethodInstInfo& );
        ControlMethodInstInfo( const ControlMethod::InstInfo& );
    };

    class ControlMethodLine {
        ControlMethod::MethodLine line_;
    public:
        ControlMethodLine();
        ControlMethodLine( const ControlMethodLine& );
        ControlMethodLine( const ControlMethod::MethodLine& );
    };

    class ControlMethodHelper {
        ControlMethod::Method method_;
    public:
        ControlMethodHelper();
        ControlMethodHelper( const ControlMethod::Method& );
        ControlMethodHelper( const ControlMethodHelper& );

        const wchar_t * subject() const ;
        void subject( const std::wstring& );

        const wchar_t * description() const;
        void description( const std::wstring& );

        inline operator const ControlMethod::Method& () const { return method_; }
        bool add( const ControlMethodInstInfo& );
        bool add( const ControlMethodLine& );
    };
}

#endif // CONTROLMETHODHELPER_HPP
