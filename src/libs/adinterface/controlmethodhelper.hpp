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

#ifndef CONTROLMETHODHELPER_HPP
#define CONTROLMETHODHELPER_HPP

#include "controlmethodC.h"

namespace adinterface {

    class Method; // C++ class

    class ControlMethodInstInfo {
        ControlMethod::InstInfo& info_;
    public:
        ControlMethodInstInfo( ControlMethod::InstInfo& );
        unsigned long index() const;
        unsigned long unit_number() const; // 0..n
        ControlMethod::eDeviceCategory category() const;
        const wchar_t * modelname() const;
        const wchar_t * serial_number() const;
        const wchar_t * description() const;

        void unit_number( unsigned long );
        void category( ControlMethod::eDeviceCategory );
        void modelname( const std::wstring& );
        void serial_number( const std::wstring& );
        void description( const std::wstring& );
    };

    class ControlMethodLine {
        ControlMethod::MethodLine& line_;
    public:
        ControlMethodLine( ControlMethod::MethodLine& );
        unsigned long index() const;
        const wchar_t * modelname() const;
        void modelname( const std::wstring& modelname );
        unsigned long unitnumber() const;
        void unitnumber( unsigned long );
        bool isInitialCondition() const;
        void isInitialCondition( bool );
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
        unsigned int findInstrument( const std::wstring& modelname, unsigned long unitnumber = 0 );
        ControlMethod::InstInfo& addInstrument( const std::wstring& modelname, unsigned long unitnumber = 0 );
        ControlMethod::MethodLine& add( const std::wstring& modelname, unsigned long unitnumber = 0 );

        static unsigned int findInstrument( const ControlMethod::Method&, const std::wstring& modelname, unsigned long unitnumber = 0 );
        static const ControlMethod::MethodLine* findFirst( const ControlMethod::Method&, const std::wstring& model, unsigned long unitnumber = 0 );
        static const ControlMethod::MethodLine* findNext( const ControlMethod::Method&, const ControlMethod::MethodLine * );
        static ControlMethod::MethodLine* findFirst( ControlMethod::Method&, const std::wstring& model, unsigned long unitnumber = 0 );
        static ControlMethod::MethodLine* findNext( ControlMethod::Method&, const ControlMethod::MethodLine * );
        static bool append( ControlMethod::Method&, const ControlMethod::MethodLine&, const std::wstring& model, unsigned long unitnumber = 0 );
        static ControlMethod::MethodLine& add( ControlMethod::Method&, const std::wstring& modelname, unsigned long unitnumber = 0 );

        static bool copy( Method& dst, const ControlMethod::Method& src );
        static bool copy( ControlMethod::Method& dst, const Method& src );
    };
}

#endif // CONTROLMETHODHELPER_HPP
