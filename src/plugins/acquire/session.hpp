// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <adicontroller/instrument.hpp>
#include <adicontroller/signalobserver.hpp>
#include <adicontroller/constants.hpp>

namespace acquire {

    class session : public adicontroller::Instrument::Session {
    public:
        ~session();
        session();
      
        std::string software_revision() const override;

        bool setConfiguration( const std::string& xml ) override;
        bool configComplete() override;
        
        bool connect( adicontroller::Receiver * , const std::string& token ) override;
        bool disconnect( adicontroller::Receiver * ) override;

        uint32_t get_status() override;
        adicontroller::SignalObserver::Observer * getObserver(void) override;
        
        bool initialize() override;
        
        bool shutdown() override;
        
        bool echo( const std::string& msg ) override;
        bool shell( const std::string& cmdline ) override;

        std::shared_ptr< const adcontrols::ControlMethod::Method > getControlMethod() override;
        bool prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > m ) override;

        // bool push_back( SampleBroker::SampleSequence_ptr s ) override;
        bool event_out( uint32_t value ) override;
        bool start_run() override;
        bool suspend_run() override;
        bool resume_run() override;
        bool stop_run() override;
        bool recording( bool ) override { return false; }
        bool isRecording() const override { return false; }

        // CORBA::Char * running_sample() override;
      
    private:

    };

}
