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

#pragma once

#include <boost/uuid/uuid.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace adfs { class sqlite; }

namespace adutils {

    // data created by adicontroller namespace class
    
    namespace v3 {

        class AcquiredConf {
        public:
            AcquiredConf();

            struct data {
                data();
                data( const data& );
                boost::uuids::uuid objid;
                std::string objtext;              // source text for create uuid
                boost::uuids::uuid pobjid;
                std::string dataInterpreterClsid;
                int trace_method;
                int spectrometer;
                std::string trace_id;
                std::wstring trace_display_name;
                std::wstring axis_label_x;
                std::wstring axis_label_y;
                int axis_decimals_x;
                int axis_decimals_y;
            };
            
            static bool insert( adfs::sqlite& dbf, const boost::uuids::uuid&& objid, const data& d );

            static bool insert( adfs::sqlite& dbf
                                , const boost::uuids::uuid& objid
                                , const std::string& objtext
                                , const boost::uuids::uuid& parent_objid
                                , const std::string& dataInterpreterClsid
                                , int trace_method
                                , int spectrometer
                                , const std::string& trace_id
                                , const std::wstring& trace_display_name
                                , const std::wstring& axis_label_x
                                , const std::wstring& axis_label_y
                                , int axis_decimals_x
                                , int axis_decimals_y );
            
            static bool fetch( adfs::sqlite& dbf, std::vector< data >& );
            static bool create_table_v3( adfs::sqlite& db );
        };

    }
}


