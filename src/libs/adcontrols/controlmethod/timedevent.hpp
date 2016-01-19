// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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
            typedef adcontrols::ControlMethod::EventCap::value_type value_type;
            
            ~TimedEvent();
            TimedEvent();
            TimedEvent( const TimedEvent& );

            TimedEvent( const ModuleCap&, const EventCap&, const value_type& );

            void setModelClsid( const boost::uuids::uuid& );
            const boost::uuids::uuid& modelClsid() const;

            double time() const;
            void setTime( double seconds );

            const value_type& value() const;
            void setValue( const value_type& );

        private:
            boost::uuids::uuid clsid_; // model class id
            std::string item_name_;
            std::string model_display_name_;
            std::string item_display_name_;
            double time_;
            value_type value_;

            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int version );
            friend class TimedEvent_archive< TimedEvent >;
            friend class TimedEvent_archive< const TimedEvent >;
        };
    }
}
