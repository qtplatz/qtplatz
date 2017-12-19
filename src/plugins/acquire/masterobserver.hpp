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

#include <adacquire/signalobserver.hpp>
#include <string>

namespace acquire {

    namespace so = adacquire::SignalObserver;

    class MasterObserver : public adacquire::SignalObserver::Observer {

        class impl;
        impl * impl_;

    public:

        MasterObserver();
        ~MasterObserver();

        bool connect( so::ObserverEvents * cb, so::eUpdateFrequency frequency, const std::string& token ) override;
        bool disconnect( so::ObserverEvents * cb ) override;

        const boost::uuids::uuid& objid() const override;
        const char * objtext() const override;

        bool addSibling( Observer * observer ) override;

        uint64_t uptime() const override { return 0; }
        void uptime_range( uint64_t& oldest, uint64_t& newest ) const override { oldest = newest = 0; }

        std::shared_ptr< so::DataReadBuffer > readData( uint32_t pos ) override { return 0; }
        const char * dataInterpreterClsid() const override             { return ""; }

        int32_t posFromTime( uint64_t usec ) const override            { return 0; }
        bool prepareStorage( adacquire::SampleProcessor& ) const override { return false; }
        bool closingStorage( adacquire::SampleProcessor& ) const override { return false; }

        // local impl
        void dataChanged( adacquire::SignalObserver::Observer *, uint32_t pos );
    };
}
