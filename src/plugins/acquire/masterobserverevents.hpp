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

#include <adacquire/signalobserver.hpp>
#include <adacquire/constants.hpp>
#include <memory>

namespace adextension { class iController; }

namespace acquire {

    class MasterController;

    /*
     *  ObserverEvents -- implementation helper
     */

    class MasterObserverEvents : public adacquire::SignalObserver::ObserverEvents {
        // ObserverEvents
        std::weak_ptr< adextension::iController > controller_;

    public:
        ~MasterObserverEvents();

        MasterObserverEvents( MasterController * );
            
        void OnConfigChanged( uint32_t objId, adacquire::SignalObserver::eConfigStatus status );
            
        void OnUpdateData( uint32_t objId, long pos );
            
        void OnMethodChanged( uint32_t objId, long pos );
            
        void OnEvent( uint32_t objId, uint32_t event, long pos );
            
        void onDataChanged( adacquire::SignalObserver::Observer * so, uint32_t pos ) override;
    };

}
