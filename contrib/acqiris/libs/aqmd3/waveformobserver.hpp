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

#include "constants.hpp"
#include <adacquire/signalobserver.hpp>
#include <aqmd3controls/waveform.hpp>
#include <aqmd3controls/method.hpp>
#include <boost/uuid/uuid.hpp>
#include <deque>

namespace aqmd3 {

    namespace so = adacquire::SignalObserver;

    class WaveformObserver : public so::Observer {
        WaveformObserver( const WaveformObserver& ) = delete;
    public:
        WaveformObserver();
        virtual ~WaveformObserver();

        constexpr static const boost::uuids::uuid __objid__ = waveform_observer;        // 99075cc1-16fb-4afc-9f93-01280dd19fe9
        constexpr static const char * __objtext__           = waveform_observer_name;   // "1.aqmd3.ms-cheminfo.com"
        constexpr static const char * __data_interpreter__  = waveform_datainterpreter; // "86a34e56-66ec-4bf2-89fe-1339bb9a0ba9"

        const boost::uuids::uuid& objid() const override;
        const char * objtext() const override;

        uint64_t uptime() const override;
        void uptime_range( uint64_t& oldest, uint64_t& newest ) const override;

        std::shared_ptr< so::DataReadBuffer > readData( uint32_t pos ) override;

        const char * dataInterpreterClsid() const override { return "aqmd3.deprecated"; }
        int32_t posFromTime( uint64_t usec ) const override;
        bool prepareStorage( adacquire::SampleProcessor& ) const override { return false; }
        bool closingStorage( adacquire::SampleProcessor& ) const override { return false; }

        // WaveformObserver
        typedef std::pair< std::shared_ptr< aqmd3controls::waveform >, std::shared_ptr< aqmd3controls::waveform > > waveform_pair_t;
        typedef std::pair< std::shared_ptr< const aqmd3controls::waveform >, std::shared_ptr< const aqmd3controls::waveform > > const_waveform_pair_t;

        uint32_t operator << ( const_waveform_pair_t& );

    private:
        std::vector< std::shared_ptr< so::DataReadBuffer > > que_;
        const boost::uuids::uuid objid_;

    };
}
