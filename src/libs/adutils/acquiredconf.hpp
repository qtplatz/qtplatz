/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef ACQUIREDCONF_HPP
#define ACQUIREDCONF_HPP

#include <boost/uuid/uuid.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace adfs { class sqlite; }


namespace adutils {

    enum AcquiredFormatVersion { format_v2, format_v3 };

    class AcquiredConf {
    public:
        AcquiredConf();

        struct data {
            data();
            data( const data& );
            uint64_t objid;
            uint64_t pobjid;
            boost::uuids::uuid uuid;
            uint64_t trace_method;  // SignalObserver::eTRACE_METHOD
            uint64_t spectrometer;  // SignalObserver::eSPECTROMETER
            std::wstring dataInterpreterClsid;
            std::wstring trace_id;
            std::wstring trace_display_name;
            std::wstring axis_x_label;
            std::wstring axis_y_label;
            uint64_t axis_x_decimals;
            uint64_t axis_y_decimals;
        };

        static bool insert( adfs::sqlite& dbf
                            , uint64_t objid
                            , uint64_t pobjid
                            , const std::wstring& dataInterpreterClsid
                            , uint64_t trace_method
                            , uint64_t spectrometer_type
                            , const std::wstring& trace_id
                            , const std::wstring& trace_display_name
                            , const std::wstring& axis_x_label
                            , const std::wstring& axis_y_label
                            , uint64_t axis_x_decimals
                            , uint64_t axis_y_decimale );
        
        static bool insert( adfs::sqlite& dbf
                            , uint32_t objid
                            , uint32_t pobjid
                            , const boost::uuids::uuid& uuid
                            , const std::string& dataInterpreterClsid
                            , uint32_t trace_method
                            , uint32_t spectrometer_type
                            , const char * trace_id
                            , const char * trace_display_name
                            , const char * axis_x_label
                            , const char * axis_y_label
                            , uint32_t axis_x_decimals
                            , uint32_t axis_y_decimale );
        
        static AcquiredFormatVersion formatVersion( adfs::sqlite& dbf );

        static bool insert( adfs::sqlite& dbf, const data& );

        static bool fetch( adfs::sqlite& dbf, std::vector< data >& );

        static bool create_table( adfs::sqlite& db );
    };

}

#endif // ACQUIREDCONF_HPP
