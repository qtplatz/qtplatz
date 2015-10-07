/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include <adicontroller/signalobserver.hpp>
#include <string>

namespace u5303a_controller {

    namespace so = adicontroller::SignalObserver;

    class MasterObserver : public adicontroller::SignalObserver::Observer {
        adicontroller::SignalObserver::Description desc_;
        class impl;
        impl * impl_;
    public:
        MasterObserver();
        ~MasterObserver();

        bool connect( so::ObserverEvents * cb, so::eUpdateFrequency frequency, const std::string& token ) override;
        bool disconnect( so::ObserverEvents * cb ) override;

        const boost::uuids::uuid& objid() const;
        const char * objtext() const;

        uint64_t uptime() const { return 0; }
        void uptime_range( uint64_t& oldest, uint64_t& newest ) const  { oldest = newest = 0; }

        std::shared_ptr< so::DataReadBuffer > readData( uint32_t pos ) { return 0; }
        const char * dataInterpreterClsid() const                      { return ""; } // master is not responcible to any specific data class
        int32_t posFromTime( uint64_t usec ) const                     { return 0; }

        // local impl
        void dataChanged( adicontroller::SignalObserver::Observer *, uint32_t pos );
    };
}
