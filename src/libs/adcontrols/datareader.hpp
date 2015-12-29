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

#include "adcontrols_global.h"
#include <functional>
#include <memory>

namespace boost { namespace uuids { struct uuid; } }
namespace adfs { class filesystem; }

namespace adcontrols {

    class DataInterpreter;
    class datafile;
    class Chromatogram;

	class ADCONTROLSSHARED_EXPORT DataReader {

        DataReader( const DataReader& ) = delete;  // noncopyable
        DataReader& operator = (const DataReader&) = delete;

    public:
        virtual ~DataReader(void);
        DataReader( const char * traceid = nullptr );
        DataReader( adfs::filesystem&, const char * traceid = nullptr );

        enum TimeSpec { ElapsedTime, EpochTime };

        virtual bool initialize( adfs::filesystem&, const boost::uuids::uuid&, const std::string& objtxt = "" ) { return false; }
        virtual void finalize() { return ; }
        virtual size_t fcnCount() const { return 0; }
        virtual std::shared_ptr< const adcontrols::Chromatogram > TIC( int fcn ) const { return nullptr; }
        virtual int64_t findPos( double seconds, bool closest = true, TimeSpec ts = ElapsedTime ) const = 0;

        //////////////////////////////////////////////////////////////
        // singleton interfaces
        typedef std::shared_ptr< DataReader >( factory_type )( const char * traceid );
        
        static std::shared_ptr< DataReader > make_reader( const char * traceid );

        static void register_factory( std::function< factory_type >, const char * clsid );

        static void assign_reader( const char * clsid, const char * traceid );

    private:
        class impl;

    };

}

