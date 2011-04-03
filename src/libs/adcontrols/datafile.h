// This is a -*- C++ -*- header.
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

// datafile should be corresponding to single sample

#pragma once

#include "adcontrols_global.h"
#include <string>
#include <boost/any.hpp>

namespace portfolio { class Portfolio; }

namespace adcontrols {
    
    class dataSubscriber;

    class ADCONTROLSSHARED_EXPORT datafile { // visitable
    public:
        datafile(void) {}
        virtual ~datafile(void) {}

        typedef datafile * (*factory_type)(void);
        virtual factory_type factory() = 0;
        //------
        const std::wstring& filename() const;
        bool readonly() const;
        // ----- virtual methods -----
        // 'path' parameter may accept either /Acquire and /Processed with following sub-dir structures
        // data read operations
        virtual void accept( dataSubscriber& ) const = 0; // visitable
        virtual boost::any fetch( const std::wstring& path, const std::wstring& dataType ) const = 0;

        // data update, modify operations
        virtual bool saveContents( const std::wstring&, const portfolio::Portfolio&, const datafile& ) { return false; }
        virtual bool update( const std::wstring&, boost::any& ) { return false; }
        //---------

        static bool access( const std::wstring& filename );
        static datafile * create( const std::wstring& filename );
        static datafile * open( const std::wstring& filename, bool readonly = false );
        static void close( datafile *& );

    private:
        std::wstring filename_;
        bool readonly_;
    };

}

