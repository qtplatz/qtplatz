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

// datafile should be corresponding to single sample

#pragma once

#include "adcontrols_global.h"
#include <string>
#include <boost/any.hpp>

namespace portfolio { class Portfolio; }

namespace adcontrols {
    
    class dataSubscriber;
    class MSCalibrateResult;

    class ADCONTROLSSHARED_EXPORT datafile { // visitable
    public:
        datafile(void);
        virtual ~datafile(void);

        typedef datafile * (*factory_type)(void);
        virtual factory_type factory() = 0;
        //------
        bool readonly() const;
        // ----- virtual methods -----
        // 'path' parameter may accept either /Acquire and /Processed with following sub-dir structures
        // data read operations
		virtual const std::wstring& filename() const;
        virtual void accept( dataSubscriber& ) = 0; // visitable
        virtual boost::any fetch( const std::wstring& path, const std::wstring& dataType ) const = 0;
        //
        virtual int dataformat_version() const { return 0; }

        // data update, modify operations
        virtual bool saveContents( const std::wstring&, const portfolio::Portfolio&, const datafile& );
        virtual bool saveContents( const std::wstring&, const portfolio::Portfolio& );
		virtual bool loadContents( const std::wstring& /* path */, const std::wstring& /* id */, dataSubscriber& ) { return false; }
        //---------

        virtual bool applyCalibration( const std::wstring&, const MSCalibrateResult& ) { return false; }
        virtual bool readCalibration( size_t idx, MSCalibrateResult& ) const { (void)idx; return false; }

        static bool access( const std::wstring& filename );
        static datafile * create( const std::wstring& filename );
        static datafile * open( const std::wstring& filename, bool readonly = false );
        static void close( datafile *& );

    private:
#ifdef _MSC_VER
# pragma warning( disable: 4251 ) // dll-linkage for
#endif
        std::wstring filename_;
        bool readonly_;
    };

}

