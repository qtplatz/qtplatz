/**************************************************************************
** Copyright (C) 2014-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adacquire/receiver.hpp>
#include <adacquire/constants.hpp>
#include <memory>

namespace adextension { class iController; }

namespace acquire {

    class MasterController;

    class MasterReceiver : public adacquire::Receiver {

        std::weak_ptr< adextension::iController > controller_;

    public:
        ~MasterReceiver();

        MasterReceiver( MasterController * p );
            
        void message( eINSTEVENT msg, uint32_t value ) override;
            
        void log( const adacquire::EventLog::LogMessage& log ) override;
            
        void shutdown() override;
            
        void debug_print( uint32_t priority, uint32_t category, const std::string& text ) override;
        
    };

}
