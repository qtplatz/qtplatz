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
#include <adwidgets/centroidform.hpp>
#include <adwidgets/progresswnd.hpp>
#include <adportfolio/logging_hook.hpp>
#include <adportfolio/folium.hpp>
#include <qtwrapper/application.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <pugixml.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/mimedatabase.h>
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

using namespace lipidid;

LipididPlugin::~LipididPlugin()
{
    if ( mode_ )
        removeObject( mode_.get() );
}

LipididPlugin::LipididPlugin() : mainWindow_( new MainWindow )
{
}

bool
LipididPlugin::initialize( const QStringList& arguments, QString* error_message )
{
    Q_UNUSED( arguments );

    Core::ICore * core = Core::ICore::instance();
    if ( core == 0 )
        return false;

    do {
        //------------------------------------------------
        if ( !Core::MimeDatabase::addMimeTypes( ":/lipidid/mimetype.xml", error_message ) )
            ADWARN() << "addMimeTypes" << ":/lipidid/mimetype.xml" << error_message;
    } while ( 0 );

    if (( mode_ = std::make_unique< lipidid::Mode >( this ) )) {
        mainWindow_->activateLayout();
        QWidget * widget = mainWindow_->createContents( mode_.get() );
        widget->setObjectName( QLatin1String( "Lipidid") );
        mode_->setWidget( widget );
        addObject( mode_.get() );
    } else {
        ADWARN() << "lipidid::Mode allocation failed.";
        return false;
    }
    return true;
}

void
LipididPlugin::extensionsInitialized()
{
    mainWindow_->OnInitialUpdate();
    Core::ModeManager::activateMode( mode_->id() );
}


ExtensionSystem::IPlugin::ShutdownFlag
LipididPlugin::aboutToShutdown()
{
    document::instance()->finalClose();
    mainWindow_->OnFinalClose();
#if ! defined NDEBUG
    ADDEBUG() << "## Shutdown: "
              << "\t" << boost::filesystem::relative( boost::dll::this_line_location()
                                                     , boost::dll::program_location().parent_path() );
#endif
	return SynchronousShutdown;
}

Q_EXPORT_PLUGIN( LipididPlugin )
