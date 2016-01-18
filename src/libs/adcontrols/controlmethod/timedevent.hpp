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

#include "adcontrols_global.h"
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

        template<typename T> class TimedEvent_archive;

        class ADCONTROLSSHARED_EXPORT TimedEvent {
        public:
            ~TimedEvent();
            TimedEvent();
            TimedEvent( const TimedEvent& );

            typedef std::pair<double, double> delay_width_type;
            typedef std::string formula_type;
            typedef boost::variant< bool, uint32_t, uint64_t, double, formula_type, delay_width_type > value_type;

            void setModelClsid( const boost::uuids::uuid& );
            const boost::uuids::uuid& modelClsid() const;

            double time() const;
            void setTime( double seconds );

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
