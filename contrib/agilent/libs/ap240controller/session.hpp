/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include <adacquire/instrument.hpp>
#include <adacquire/signalobserver.hpp>

//namespace mpxcontrols { class ControlMethod; }
namespace ap240 { class method; class digitizer; }

namespace ap240controller {
    
    namespace Instrument {

        // Session class define here is psude singletion by a manager class
        // which is only the class make Session instance.
        
        class Session : public adacquire::Instrument::Session {
            Session( const Session& ) = delete;
            Session& operator = ( const Session& ) = delete;
            struct impl;
            impl * impl_;
        public:
            // exception
            struct CannotAdd { std::string reason_; };

            static Session * instance();

            Session();
            ~Session();

            std::string software_revision() const override;  // ex. L"1.216"

            bool setConfiguration( const std::string& xml ) override;
            bool configComplete() override;
            
            bool connect( adacquire::Receiver * receiver, const std::string& token ) override;
            bool disconnect( adacquire::Receiver * receiver ) override;
      
            uint32_t get_status() override;
            adacquire::SignalObserver::Observer * getObserver() override;
      
            bool initialize() override;

            bool shutdown() override;  // shutdown server
            bool echo( const std::string& msg ) override;
            bool shell( const std::string& cmdline ) override;
            std::shared_ptr< const adcontrols::ControlMethod::Method > getControlMethod() override;
            bool prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > m ) override;
            bool time_event_trigger( std::shared_ptr< const adcontrols::ControlMethod::TimedEvents > tt
                                     , adcontrols::ControlMethod::const_time_event_iterator begin
                                     , adcontrols::ControlMethod::const_time_event_iterator end ) override;

            bool event_out( uint32_t event ) override;
            bool start_run() override;
            bool suspend_run() override;
            bool resume_run() override;
            bool stop_run() override;
            bool recording( bool ) override;
            bool isRecording() const override;
            
        };

    };
    
} // namespace adicontroler
