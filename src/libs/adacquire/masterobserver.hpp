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

#include "adacquire_global.hpp"
#include "signalobserver.hpp"

namespace boost { namespace uuids { struct uuid; } }

namespace adacquire {

    namespace so = adacquire::SignalObserver;

    class ADACQUIRESHARED_EXPORT MasterObserver : public SignalObserver::Observer {

        class impl;
        impl * impl_;

    public:

        MasterObserver( const char * objtext = 0 );
        ~MasterObserver();

        bool connect( so::ObserverEvents * cb, so::eUpdateFrequency, const std::string& ) override;
        bool disconnect( so::ObserverEvents * cb ) override;

        const boost::uuids::uuid& objid() const override;
        const char * objtext() const override;

        uint64_t uptime() const override;
        std::shared_ptr< so::DataReadBuffer > readData( uint32_t pos ) override;
        const char * dataInterpreterClsid() const override;

        bool prepareStorage( SampleProcessor& ) const override;
        bool closingStorage( SampleProcessor& ) const override;

        //
        void setPrepareStorage( std::function< bool( SampleProcessor& ) > );
        void setClosingStorage( std::function< bool( SampleProcessor& ) > );

        virtual void dataChanged( SignalObserver::Observer * so, uint32_t pos );

    private:
        std::function< bool( SampleProcessor& ) > preparing_;
        std::function< bool( SampleProcessor& ) > closing_;
    };

}
