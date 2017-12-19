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

#include "mastercontroller.hpp"
#include "document.hpp"
#include "masterreceiver.hpp"
#include "masterobserverevents.hpp"
#include "session.hpp"
#include "task.hpp"
#include <adplugin/plugin.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adacquire/instrument.hpp>
#include <adacquire/receiver.hpp>
#include <adacquire/signalobserver.hpp>
#include <adportable/debug.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportable/serializer.hpp>
#include <adacquire/manager.hpp>
#include <QLibrary>
#include <QVariant>
#include <atomic>
#include <memory>

namespace acquire {

    class MasterController::impl {
    public:
        
        impl() {
            connected_.clear();
        }

        std::atomic_flag connected_;
        std::once_flag flag_;
    };
    
}

using namespace acquire;

MasterController::MasterController() : adextension::iControllerImpl( "Acquire" )
                                     , impl_( new impl() )
{
}

MasterController::~MasterController()
{
}

bool
MasterController::connect()
{
    // Press 'Connect' button on Acquire's MainWindow, this method call from document::actionConnect();
    // Press 'Connect' on other plugin's view, this call back from iController::connect invoker.

    std::call_once( impl_->flag_, [this] () { session_ = std::make_shared< session >(); } );

    if ( impl_->connected_.test_and_set( std::memory_order_acquire ) == false ) {

        document::instance()->actionConnect( false ); // call document's actionConnect
        setInitialized( true );

        emit connected( this ); // --> MainWindow::iControllerConnected; on UI thread

    }
    return true;

}

