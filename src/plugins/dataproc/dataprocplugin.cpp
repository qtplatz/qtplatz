// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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
#if QTC_VERSION < 0x09'00'00
# define EXCLUDE 1
#else
# define EXCLUDE 0
#endif
#include "dataprocplugin.hpp"
#include "actionmanager.hpp"
#include "constants.hpp"
#include "document.hpp"
#if EXCLUDE
#include "dataprocessor.hpp"
#include "dataproceditor.hpp"
#endif

#include "dataprocfactory.hpp"

#include <adextension/isequenceimpl.hpp>
#include "isnapshothandlerimpl.hpp"
#include "ipeptidehandlerimpl.hpp"
#include "mainwindow.hpp"
#if EXCLUDE
#include "mimetypehelper.hpp"
#endif

#include "mode.hpp"
#include "navigationwidgetfactory.hpp"
#include "sessionmanager.hpp"
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
#include <pugixml.hpp>

#include <coreplugin/icore.h>
#if EXCLUDE
#include <coreplugin/id.h>
#else
#include <utils/id.h>
#endif
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#if EXCLUDE
#include <coreplugin/mimedatabase.h>
#else
#include <utils/mimetypes/mimedatabase.h>
#endif
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

namespace dataproc {

    class DataprocPlugin::impl {
    public:
        impl( DataprocPlugin * p ) : pThis_( p )
                                   , editorFactory_( std::make_unique< DataprocFactory >() ) {
        }
        DataprocPlugin * pThis_;

        // self-registratoin in ctor (see L203, coreplugin/editormanager/iedtorfactory.cpp )
        std::unique_ptr< DataprocFactory > editorFactory_;

        std::unique_ptr< ActionManager > actionManager_;
        std::unique_ptr< MainWindow > mainWindow_;
        std::unique_ptr< dataproc::Mode > mode_;

        std::unique_ptr< iSnapshotHandlerImpl > iSnapshotHandler_;
        std::unique_ptr< iPeptideHandlerImpl > iPeptideHandler_;
        std::unique_ptr< NavigationWidgetFactory > navigationWidgetFactory_;

        bool ini() {
            if (( mode_ = std::make_unique< dataproc::Mode >( pThis_ ) )) {
                actionManager_->initialize_actions( mode_->context() );
                mainWindow_->activateLayout();
                QWidget * widget = mainWindow_->createContents( mode_.get() );
                widget->setObjectName( QLatin1String( "DataprocView") );
                mode_->setWidget( widget );

                if (( iSnapshotHandler_ = std::make_unique< iSnapshotHandlerImpl >() )) {
                    connect_isnapshothandler_signals();
                    ExtensionSystem::PluginManager::addObject( iSnapshotHandler_.get() );
                }
                if (( navigationWidgetFactory_ = std::make_unique< NavigationWidgetFactory >() )) {
                    ExtensionSystem::PluginManager::addObject( navigationWidgetFactory_.get() );
                }
                if (( iPeptideHandler_ = std::make_unique< iPeptideHandlerImpl >() )) {
                    ExtensionSystem::PluginManager::addObject( iPeptideHandler_.get() );
                }
                return true;
            }
            return false;
        }
        void fin() {
            ExtensionSystem::PluginManager::removeObject( iPeptideHandler_.get() );
            ExtensionSystem::PluginManager::removeObject( iSnapshotHandler_.get() );
            ExtensionSystem::PluginManager::removeObject( navigationWidgetFactory_.get() );
        }

    private:
        // this may move to document ???
        void connect_isnapshothandler_signals() {
            auto p = DataprocPlugin::instance();
            p->connect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onPortfolioCreated, p, [] ( const QString& _1 ) {
                document::instance()->handle_portfolio_created( _1 );
            } );
            p->connect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onFoliumAdded, p, [] ( const QString& _1, const QString& _2, const QString& _3 ) {
                document::instance()->handle_folium_added( _1, _2, _3 );
            } );
        }
    };

    DataprocPlugin * DataprocPlugin::instance_ = 0;
}



using namespace dataproc;

DataprocPlugin::~DataprocPlugin()
{
}

DataprocPlugin::DataprocPlugin() : impl_( std::make_unique< impl >( this ) )
{
    instance_ = this;
    impl_->mainWindow_ = std::make_unique< MainWindow >();
    impl_->actionManager_ = std::make_unique< ActionManager >( this );
}

//static
DataprocPlugin *
DataprocPlugin::instance()
{
    return instance_;
}

//static
MainWindow *
DataprocPlugin::mainWindow()
{
    return instance()->impl_->mainWindow_.get();
}

ActionManager *
DataprocPlugin::actionManager()
{
    return impl_->actionManager_.get();
}

bool
DataprocPlugin::initialize( const QStringList& arguments, QString* error_message )
{
    Q_UNUSED( arguments );

    Core::ICore * core = Core::ICore::instance();
    if ( core == 0 )
        return false;

    if ( ! impl_->ini() )
        return false;

    return true;
}

void
DataprocPlugin::applyMethod( const adcontrols::ProcessMethod& m )
{
	emit onApplyMethod( m );
}

void
DataprocPlugin::extensionsInitialized()
{
    impl_->mainWindow_->OnInitialUpdate();
    document::instance()->initialSetup();
    Core::ModeManager::activateMode( impl_->mode_->id() );
}


ExtensionSystem::IPlugin::ShutdownFlag
DataprocPlugin::aboutToShutdown()
{
    document::instance()->finalClose();

    impl_->mainWindow_->OnFinalClose();
    impl_->fin();

#if ! defined NDEBUG
    ADDEBUG() << "## Shutdown: "
              << "\t" << boost::filesystem::relative( boost::dll::this_line_location()
                                                     , boost::dll::program_location().parent_path() );
#endif

	return SynchronousShutdown;
}

//////////////


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_EXPORT_PLUGIN( DataprocPlugin )
#endif
