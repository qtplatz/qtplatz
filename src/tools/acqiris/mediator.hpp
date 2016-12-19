/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include <memory>
#include <mutex>

namespace acqrscontrols {
namespace aqdrv4 {

    class acqiris_client;
    class acqiris_method;
    class waveform;

    enum SubMethodType : unsigned int;
    namespace server {
        class tcp_server;
    }
    namespace client {
        class tcp_client;
    }
}
}

namespace acqrscontrols { namespace aqdrv4 { class acqiris_method; } }

class mediator {
protected:
    mediator();
public:
    ~mediator();

    static mediator * instance();

    // method sent from client -> server
    virtual void prepare_for_run( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > p, acqrscontrols::aqdrv4::SubMethodType t );

    // event in from client -> out to server
    virtual void eventOut( uint32_t e );

    // method adapted from server -> client
    virtual void acqiris_method_adapted( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > p );

    // temperature data from server -> cleint
    virtual void replyTemperature( int t );
    
    std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method > acqiris_method();
    void set_acqiris_method( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > );
private:
    std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > adapted_method_;
    std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > method_;
};

