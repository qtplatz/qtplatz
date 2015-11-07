// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

// #include "acquireplugin.hpp"
// #include "acquiremode.hpp"
// #include "brokerevent_i.hpp"
// #include "constants.hpp"
// #include "document.hpp"
// #include "mastercontroller.hpp"
// #include "mainwindow.hpp"
// #if HAVE_CORBA
// #include "orbconnection.hpp"
// #include "qbroker.hpp"
// #endif

// #include <acewrapper/constants.hpp>
// #include <acewrapper/ifconfig.hpp>
// #include <adextension/isnapshothandler.hpp>

#if HAVE_CORBA
# include "receiver_i.hpp"
# include <adinterface/brokerC.h>
# include <adinterface/controlserverC.h>
# include <adinterface/receiverC.h>
// # include <adinterface/signalobserverC.h>
# include <adinterface/observerevents_i.hpp>
// # include <adinterface/eventlog_helper.hpp>
// # include <adinterface/controlmethodhelper.hpp>
// # include <adorbmgr/orbmgr.hpp>
// # include <tao/Object.h>
#endif

// #include <adcontrols/controlmethod.hpp>
// #include <adcontrols/massspectrum.hpp>
// #include <adcontrols/msproperty.hpp>
// #include <adcontrols/description.hpp>
// #include <adcontrols/descriptions.hpp>
// #include <adcontrols/massspectrometer.hpp>
// #include <adcontrols/mscalibrateresult.hpp>
// #include <adcontrols/msreference.hpp>
// #include <adcontrols/msreferences.hpp>
// #include <adcontrols/mscalibration.hpp>
// #include <adcontrols/msassignedmass.hpp>
// #include <adcontrols/datainterpreter.hpp>
// #include <adcontrols/centroidprocess.hpp>
// #include <adcontrols/centroidmethod.hpp>
// #include <adcontrols/controlmethod.hpp>
// #include <adcontrols/samplerun.hpp>
// #include <adcontrols/trace.hpp>
// #include <adcontrols/traceaccessor.hpp>
// #include <adcontrols/timeutil.hpp>
// #include <adextension/imonitorfactory.hpp>
// #include <adextension/icontroller.hpp>
// #include <adportable/array_wrapper.hpp>
// #include <adportable/configuration.hpp>
// #include <adportable/configloader.hpp>
// #include <adportable/date_string.hpp>
// #include <adportable/profile.hpp>
// #include <adportable/binary_serializer.hpp>
// #include <adportable/debug.hpp>
// #include <adplugin_manager/loader.hpp>
// #include <adplugin_manager/manager.hpp>
// #include <adplugin/plugin.hpp>
// #include <adplugin/plugin_ptr.hpp>
// #include <adplugin/orbbroker.hpp>

// #include <adportable/debug_core.hpp>
// #include <adlog/logging_handler.hpp>
// #include <adlog/logger.hpp>

// #include <adportable/date_string.hpp>
// #include <adportable/fft.hpp>
// #include <adportable/debug.hpp>
// #include <adplot/chromatogramwidget.hpp>
// #include <adplot/spectrumwidget.hpp>

// #include <qtwrapper/application.hpp>
// #include <qtwrapper/qstring.hpp>
// #include <servant/servantplugin.hpp>
// #include <utils/fancymainwindow.h>

// #include <coreplugin/icore.h>
// #include <coreplugin/id.h>
// #include <coreplugin/actionmanager/actioncontainer.h>
// #include <coreplugin/actionmanager/actionmanager.h>
// #include <coreplugin/coreconstants.h>
// #include <coreplugin/minisplitter.h>
// #include <coreplugin/modemanager.h>
// #include <coreplugin/navigationwidget.h>
// #include <coreplugin/outputpane.h>
// #include <coreplugin/rightpane.h>
// #include <extensionsystem/pluginmanager.h>
// #include <utils/styledbar.h>

// #include <QAction>
// #include <QComboBox>
// #include <QtCore/qplugin.h>
// #include <QFileDialog>
// #include <QHBoxLayout>
// #include <QBoxLayout>
// #include <QToolButton>
// #include <QLabel>
// #include <QLineEdit>
// #include <QTableWidget>
// #include <QTextEdit>
// #include <QToolButton>
// #include <QMessageBox>
// #include <qdebug.h>

// #include <boost/exception/all.hpp>
// #include <boost/format.hpp>
// #include <boost/filesystem.hpp>
// #include <boost/date_time/posix_time/posix_time.hpp>
// #include <boost/bind.hpp>
// #include <boost/exception/all.hpp>
// #include <algorithm>
// #include <cmath>
// #include <fstream>
// #include <functional>
// #include <future>
#include <map>

namespace adcontrols { class MassSpectrometer; }

namespace acquire {

    class AcquirePlugin;

    class orb_i {
        AcquirePlugin * pThis_;
    public:
        ControlServer::Session_var session_;
        SignalObserver::Observer_var observer_;

        typedef std::tuple< SignalObserver::Observer_var
                            , SignalObserver::Description_var
                            , std::wstring
                            , bool
                            , std::shared_ptr< adcontrols::MassSpectrometer > > observer_type;

        std::map< unsigned long, observer_type > observerMap_;
        std::unique_ptr< receiver_i > receiver_i_;
        std::unique_ptr< adinterface::ObserverEvents_i > sink_;
        std::map< unsigned long, long > npos_map_;

        void actionConnect();
        void actionDisconnect();
        void actionInitRun();
        void actionRun();
        void actionStop();
        void actionInject();
        void actionSnapshot();
        bool readCalibrations( observer_type& );
        
        void handle_update_data( unsigned long objId, long pos );
        void handle_controller_message( unsigned long /* Receiver::eINSTEVENT */ msg, unsigned long value );
        
        orb_i( AcquirePlugin * p ) : pThis_( p ) {
        }
    };
}

