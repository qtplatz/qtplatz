// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC
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

#include "lipididplugin.hpp"
#include "constants.hpp"
#include "document.hpp"
#include "mainwindow.hpp"
#include "mode.hpp"
#include "utils/result.h"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/modemanager.h>
#include <acewrapper/constants.hpp>
#include <adcontrols/datafilebroker.hpp>
#include <adcontrols/datafile_factory.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <adextension/ieditorfactory.hpp>
#include <adextension/isnapshothandler.hpp>
#include <adextension/isessionmanager.hpp>

#include <adplugin/plugin.hpp>
#include <adplugin/constants.hpp>
#include <adplugin_manager/loader.hpp>
#include <adplugin_manager/manager.hpp>

#include <adportable/array_wrapper.hpp>
#include <adportable/configuration.hpp>
#include <adportable/configloader.hpp>
#include <adportable/debug.hpp>
#include <adportable/debug_core.hpp>
#include <adlog/logger.hpp>
#include <adlog/logging_handler.hpp>
#include <adportable/float.hpp>
#include <adwidgets/progresswnd.hpp>
#include <adportfolio/logging_hook.hpp>
#include <adportfolio/folium.hpp>
#include <qtwrapper/application.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/plugin_manager.hpp>
#include <pugixml.hpp>

#include <coreplugin/icore.h>
#include <utils/id.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
//#include <coreplugin/mimedatabase.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/navigationwidget.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>
#include <utils/fancymainwindow.h>
#include <QApplication>
#include <QStringList>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QSpacerItem>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QDir>
#include <QMessageBox>
#include <QProgressBar>
#include <QtCore/qplugin.h>
#include <QtCore>

#include <boost/dll.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/exception/all.hpp>
#include <streambuf>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <thread>

namespace {
    static constexpr char __CLASS_NAME__[] = "LipididPlugin";
}


using namespace lipidid;

LipididPlugin::~LipididPlugin()
{
}

LipididPlugin::LipididPlugin() : mainWindow_( new MainWindow )
{
}

Utils::Result<>
LipididPlugin::initialize( const QStringList& arguments )
{
#if ! defined NDEBUG
    qDebug() << "\t#### " << __CLASS_NAME__ << "::" << __FUNCTION__ << " #### " << arguments;
#endif

    ADDEBUG() << "#### LipididPlugin::" << __FUNCTION__ << " ####";
    Core::ICore * core = Core::ICore::instance();
    if ( core == 0 )
        return Utils::ResultError( "Core instance is null" );

    if (( mode_ = std::make_unique< lipidid::Mode >( this ) )) {
        mainWindow_->activateLayout();
        mainWindow_->initializeActions( mode_.get() );
        QWidget * widget = mainWindow_->createContents( mode_.get() );
        widget->setObjectName( QLatin1String( "Lipidid") );
        mode_->setWidget( widget );
    } else {
        ADWARN() << "lipidid::Mode allocation failed.";
        return Utils::ResultError( "LipidId::Mode allocation failed" );
    }
    return Utils::ResultOk;
}

void
LipididPlugin::extensionsInitialized()
{
    mainWindow_->OnInitialUpdate();
#if ! defined NDEBUG
    ADDEBUG() << "\t#### " << __CLASS_NAME__ << "::" << __FUNCTION__ << " ####";
#endif

    // SessionManager is a singleton, instanciated in dataproc
    auto mgr = ExtensionSystem::PluginManager::instance()->getObject< adextension::iSessionManager >();
    using adextension::iSessionManager;
    connect( mgr, &iSessionManager::addProcessor, document::instance(), &document::handleAddProcessor );
    connect( mgr, &iSessionManager::onSelectionChanged, document::instance(), &document::handleSelectionChanged );
    connect( mgr, &iSessionManager::onProcessed, document::instance(), &document::handleProcessed );
    connect(mgr, &iSessionManager::onCheckStateChanged, document::instance(), &document::handleCheckStateChanged);
    connect( mgr, &iSessionManager::onSessionRemoved, document::instance(), &document::handleSessionRemoved );
    Core::ModeManager::activateMode( mode_->id() );
}


ExtensionSystem::IPlugin::ShutdownFlag
LipididPlugin::aboutToShutdown()
{
    document::instance()->finalClose();
    mainWindow_->OnFinalClose();
#if ! defined NDEBUG
    ADDEBUG() << "\t\t## " << __CLASS_NAME__ << "::" << __FUNCTION__
              << "\t" << std::filesystem::relative( boost::dll::this_line_location()
                                                     , boost::dll::program_location().parent_path() );
#endif
	return SynchronousShutdown;
}
