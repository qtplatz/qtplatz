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
#include "../controlmethod_fwd.hpp"
#include <string>
#include <map>
#include <memory>
#include <vector>

namespace boost {
    namespace serialization { class access; }
    namespace uuids { struct uuid; }
}

namespace adcontrols {

    namespace ControlMethod {

        class TimedEvent;
        template< typename T > class TimedEvents_archive;

        class ADCONTROLSSHARED_EXPORT TimedEvents {
        public:
            ~TimedEvents();
            TimedEvents();
            TimedEvents( const TimedEvents& );

            static const char * modelClass() { return "TimedEvents"; }
            static const char * itemLabel() { return "TimedEvents.1"; }
            static const boost::uuids::uuid& clsid();

            typedef time_event_iterator iterator;             // controlmethod_fwd.hpp
            typedef const_time_event_iterator const_iterator; // controlmethod_fwd.hpp

            size_t size() const;
            void clear();

            TimedEvents& operator << ( const TimedEvent& );

            iterator begin();
            iterator end();

            const_iterator begin() const;
            const_iterator end() const;

            static bool archive( std::ostream&, const TimedEvents& );
            static bool restore( std::istream&, TimedEvents& );
            static bool xml_archive( std::wostream&, const TimedEvents& );
            static bool xml_restore( std::wistream&, TimedEvents& );

        private:
            std::vector< TimedEvent > vec_;

            friend class TimedEvents_archive< TimedEvents >;
            friend class TimedEvents_archive< const TimedEvents >;

            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int version );
        };

    }
}
