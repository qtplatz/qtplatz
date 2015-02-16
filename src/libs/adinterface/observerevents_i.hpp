/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <adinterface/signalobserverS.h>
#include <functional>
#include <adportable/debug.hpp>

namespace adinterface {

    class ObserverEvents_i : public POA_SignalObserver::ObserverEvents {
    public:
        void OnConfigChanged( CORBA::ULong oid, SignalObserver::eConfigStatus status ) override {
            if ( onConfigChanged_ )
                onConfigChanged_( oid, status );
        }
        void OnUpdateData( CORBA::ULong oid, CORBA::Long pos ) override {
            if ( onUpdateData_ )
                onUpdateData_( oid, pos );
        }
        void OnMethodChanged( CORBA::ULong oid, CORBA::Long pos ) override {
            if ( onMethodChanged_ )
                onMethodChanged_( oid, pos );
        }
        void OnEvent( CORBA::ULong oid, CORBA::ULong pos, CORBA::Long ev ) override {
            if ( onEvent_ )
                onEvent_( oid, pos, ev );
        }

        void assignConfigChanged( std::function< void(uint32_t, SignalObserver::eConfigStatus) > f) {
            onConfigChanged_ = f;  
        }
        void assignUpdateData( std::function< void(uint32_t, int32_t) > f ) {
            onUpdateData_ = f;
        }
        void assignMethodChanged( std::function< void(uint32_t, int32_t) > f ) {
            onMethodChanged_ = f;
        }
        void assignEvent( std::function< void(uint32_t, int32_t, int32_t ) > f ) {
            onEvent_ = f;
        }
    private:
        std::function< void(uint32_t, SignalObserver::eConfigStatus) > onConfigChanged_;
        std::function< void(uint32_t, int32_t) > onUpdateData_;
        std::function< void(uint32_t, int32_t) > onMethodChanged_;
        std::function< void(uint32_t, int32_t, int32_t ) > onEvent_;
    };

}
