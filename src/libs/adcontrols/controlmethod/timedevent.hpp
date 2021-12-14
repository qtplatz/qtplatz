// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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

#include "../adcontrols_global.h"
#include "eventcap.hpp"
#include <boost/variant.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/serialization/version.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <memory>
#include <string>
#include <vector>

namespace boost {
    namespace serialization { class access; }
    namespace uuids { struct uuid; }
}

namespace adcontrols {
    namespace ControlMethod {

        class ModuleCap;

        template<typename T> class TimedEvent_archive;

        class ADCONTROLSSHARED_EXPORT TimedEvent {
        public:
            //typedef std::pair< double, double > delay_width_type;
            //typedef boost::variant< bool, uint32_t, uint64_t, double, delay_width_type > value_type;
            typedef EventCap::value_type value_type;

            ~TimedEvent();
            TimedEvent();
            TimedEvent( const TimedEvent& );
            TimedEvent( const ModuleCap&, const EventCap&, double time, const value_type& ); // to be deprecated

            TimedEvent( const std::string& json );
            TimedEvent& operator = ( const TimedEvent& t );

            void setModelClsid( const boost::uuids::uuid& );
            boost::uuids::uuid modelClsid() const;

            std::string modelDisplayName() const;

            std::string itemName() const;
            void setItemName( const std::string& );

            std::string itemDisplayName() const;
            void setItemDisplayName( const std::string& );

            double time() const;
            void setTime( double seconds );

            value_type value() const;
            void setValue( const value_type& );

            static std::string toString( const value_type& );

            std::string data_type() const;

            operator bool () const; // is valid for json

        private:
#if 0
            boost::uuids::uuid clsid_; // model class id
            std::string module_display_name_;
            std::string item_name_;
            std::string item_display_name_;
            double time_;
            value_type value_;
#endif
            boost::uuids::uuid modelClsid_;
            std::string modelDisplayName_;
            std::string name_; // item_name_
            std::string displayName_; // item_display_name_
            double time_;
            value_type value_;

            //
            // {
            //     "modelClsid": "522a83e8-b1b9-4341-8b0f-cac66d6d1e67",
            //     "modelDisplayName": "InfiTOF,HV",
            //     "name": "MCP-OUT.SET",
            //     "displayName": "MCP-OUT (V)",
            //     "time": "0",
            //     "data": {
            //         "type": "voltage_type",
            //         "value": "10",
            //         "limits": {
            //             "min": "0",
            //             "max": "1000"
            //         }
            //     }
            // }
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int version );
            friend class TimedEvent_archive< TimedEvent >;
            friend class TimedEvent_archive< const TimedEvent >;
            friend void tag_invoke( boost::json::value_from_tag, boost::json::value&, const TimedEvent& );
            friend TimedEvent tag_invoke( boost::json::value_to_tag< TimedEvent >&, const boost::json::value& jv );
        };

        ADCONTROLSSHARED_EXPORT
        void tag_invoke( boost::json::value_from_tag, boost::json::value&, const TimedEvent& );

        ADCONTROLSSHARED_EXPORT
        TimedEvent tag_invoke( boost::json::value_to_tag< TimedEvent >&, const boost::json::value& jv );
    }
}

BOOST_CLASS_VERSION( adcontrols::ControlMethod::TimedEvent, 2 )
