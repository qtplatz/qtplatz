/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "adextension_global.hpp"
#include <adextension/icontroller.hpp>
#include <adacquire/instrument.hpp>
#include <adacquire/receiver.hpp>
#include <adacquire/signalobserver.hpp>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <vector>
#include <memory>

class QString;

namespace adextension {

    /*
     *  Receiver -- implementation helper
     */
    class ADEXTENSIONSHARED_EXPORT ReceiverImpl : public adacquire::Receiver {

        std::weak_ptr< iController > controller_;

    public:
        ~ReceiverImpl();
        ReceiverImpl( iController * p );
        void message( eINSTEVENT msg, uint32_t value ) override;
            
        void log( const adacquire::EventLog::LogMessage& log ) override;

        void shutdown() override;
            
        void debug_print( uint32_t priority, uint32_t category, const std::string& text ) override;
        
        void notify_error( const boost::system::error_code&, const std::string& file, int line ) override;

        void notify_error( const std::string&, const std::string& file, int line ) override;
    };
}

