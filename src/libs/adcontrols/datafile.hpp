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
#include <boost/any.hpp>
#include <filesystem>
#include <string>

namespace portfolio { class Portfolio; }
namespace adfs { class sqlite; }

namespace adcontrols {

    class dataSubscriber;
    class MSCalibrateResult;
    class ADCONTROLSSHARED_EXPORT datafile;

    class datafile { // visitable
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
		virtual std::filesystem::path filename() const;
        virtual void accept( dataSubscriber& ) = 0; // visitable
        virtual boost::any fetch( const std::wstring& path, const std::wstring& dataType ) const = 0;
        virtual boost::any fetch( const std::string& path, const std::string& dataType ) const = 0;
        //
        virtual int dataformat_version() const { return 0; }

        // data update, modify operations
        virtual bool saveContents( const std::wstring&, const portfolio::Portfolio&, const datafile& ) { return false; }
        virtual bool saveContents( const std::wstring&, const portfolio::Portfolio& )                  { return false; }
		virtual bool loadContents( const std::wstring&, const std::wstring& /*id*/, dataSubscriber& )  { return false; }

        virtual bool saveContents( const std::string&, const portfolio::Portfolio&, const datafile& )  { return false; }
        virtual bool saveContents( const std::string&, const portfolio::Portfolio& )                   { return false; }
		virtual bool loadContents( const std::string&, const std::string& /*id*/, dataSubscriber& )    { return false; }
        //---------

        virtual bool applyCalibration( const std::wstring&, const MSCalibrateResult& ) { return false; }
        virtual bool readCalibration( size_t idx, MSCalibrateResult& ) const { (void)idx; return false; }

        virtual std::shared_ptr< adfs::sqlite > sqlite() const { return {}; }
        virtual bool export_rawdata( const datafile& ) const { return false; }

        static bool access( const std::filesystem::path& );
        static datafile * create( const std::filesystem::path& );
        static datafile * open( const std::filesystem::path&, bool readonly = false );
        static void close( datafile *& );

    private:
        std::filesystem::path filename_;
        bool readonly_;
    };

}
