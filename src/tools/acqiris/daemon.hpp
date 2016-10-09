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

#include "mediator.hpp"
#include <memory>
#include <mutex>

namespace acqrscontrols { namespace aqdrv4 { class acqiris_method; } }

class daemon : public mediator {
    static int __debug_mode__;
    static int __pidParent__;
    static int __pidChild__;
    daemon();
public:
    ~daemon();
    static int main( int argc, char * argv [] );
    static daemon * instance();

    void exec( const std::string& addr, const std::string& port );

    // method sent from client -> server
    void prepare_for_run( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > p, acqrscontrols::aqdrv4::SubMethodType t ) override;

    // event in from client -> out to server
    void eventOut( uint32_t e ) override;

    // method adapted from server -> client; ignore
    // void acqiris_method_adapted( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > p ) override;

    // temperature data from server -> cleint; ignore
    // void replyTemperature( int t ) override;
    
private:
    std::mutex mutex_;
};

