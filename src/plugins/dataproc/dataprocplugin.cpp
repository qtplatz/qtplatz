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

#include "dataprocplugin.hpp"
#include "actionmanager.hpp"
#include "constants.hpp"
#include "document.hpp"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "dataprocessor.hpp"
#include "dataproceditor.hpp"
#endif
#include "dataprocfactory.hpp"

#include <adextension/isequenceimpl.hpp>
#include "isnapshothandlerimpl.hpp"
#include "ipeptidehandlerimpl.hpp"
#include "mainwindow.hpp"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <coreplugin/id.h>
#else
#include <utils/id.h>
#endif
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
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
        std::unique_ptr< DataprocFactory > editorFactory_;

        std::unique_ptr< ActionManager > actionManager_;
        std::unique_ptr< MainWindow > mainWindow_;
        std::unique_ptr< dataproc::Mode > mode_;

        std::unique_ptr< adextension::iSequenceImpl > iSequence_;
        std::unique_ptr< iSnapshotHandlerImpl > iSnapshotHandler_;
        std::unique_ptr< iPeptideHandlerImpl > iPeptideHandler_;

        bool init_mode() {
            if (( mode_ = std::make_unique< dataproc::Mode >( pThis_ ) )) {
                actionManager_->initialize_actions( mode_->context() );
                mainWindow_->activateLayout();
                QWidget * widget = mainWindow_->createContents( mode_.get() );
                widget->setObjectName( QLatin1String( "DataprocessingPage") );
                mode_->setWidget( widget );
                init_handlers();
                // ExtensionSystem::PluginManager::addObject( mode_.get() ); <- may not be necessary ??
                return true;
            }
            return false;
        }

        void fin_handlers() {
            ExtensionSystem::PluginManager::removeObject( iSequence_.get() );
            ExtensionSystem::PluginManager::removeObject( iSnapshotHandler_.get() );
            ExtensionSystem::PluginManager::removeObject( iPeptideHandler_.get() );
        }

    private:
        void init_handlers() {
            if (( iSnapshotHandler_ = std::make_unique< iSnapshotHandlerImpl >() )) {
                connect_isnapshothandler_signals();
                ExtensionSystem::PluginManager::addObject( iSnapshotHandler_.get() );
            }
            if (( iPeptideHandler_ = std::make_unique< iPeptideHandlerImpl >() )) {
                ExtensionSystem::PluginManager::addObject( iPeptideHandler_.get() );
            }

            if ( adextension::iSessionManager * mgr = SessionManager::instance() ) {
                ExtensionSystem::PluginManager::addObject( mgr );
            }
        }

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
    impl_->fin_handlers();
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

    // Core::Context context( (Core::Id( Constants::C_DATAPROCESSOR )) );

    //-------------------------------------------------------------------------------------------
    std::wstring apppath = qtwrapper::application::path( L".." ); // := "~/qtplatz/bin/.."
    // adplugin::manager::standalone_initialize(); <-- already processed in servantplugin

    do {
        std::vector< std::string > mime;
        do {
            std::vector< adplugin::plugin_ptr > dataproviders;
            if ( adplugin::manager::instance()->select_iids( ".*\\.adplugins\\.datafile_factory\\..*", dataproviders ) ) {

                std::for_each( dataproviders.begin(), dataproviders.end()
                               , [&] ( const adplugin::plugin_ptr& d ) {
                                     adcontrols::datafile_factory * factory = d->query_interface< adcontrols::datafile_factory >();
                                     if ( factory ) {
                                         if ( factory->mimeTypes() )
                                             mime.push_back( factory->mimeTypes() );
                                     }
                                 } );
            }
        } while ( 0 );

        //------------------------------------------------
        QStringList mTypes;

        // core mime-types
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if ( !Core::MimeDatabase::addMimeTypes( ":/dataproc/mimetype.xml", error_message ) )
            ADWARN() << "addMimeTypes" << ":/dataproc/mimetype.xml" << error_message;
#else
        ADDEBUG() << "############### Core::MimeDatabase::addMimeTypes from xml in the resouce need to be resolved ############";
#endif
        // externally installed mime-types
        std::wstring mimefile = adplugin::loader::config_fullpath( apppath, L"/MS-Cheminformatics/dataproc-mimetype.xml" );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if ( Core::MimeDatabase::addMimeTypes( QString::fromStdWString( mimefile ), error_message ) ) {

            pugi::xml_document doc;
            if ( doc.load_file( mimefile.c_str() ) ) {
                pugi::xpath_node_set list = doc.select_nodes( "/mime-info/mime-type" );
                for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it )
                    mTypes << it->node().attribute( "type" ).value();
            }
        }

        std::for_each( mime.begin(), mime.end(), [&](const std::string& xml){
                if ( mimeTypeHelper::add( xml.c_str(), static_cast<int>(xml.size()), error_message ) )
					mimeTypeHelper::populate( mTypes, xml.c_str() );
            });
#else
        ADDEBUG() << "############### Core::MimeDatabase::addMimeTypes from xml in the resouce need to be resolved ############";
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        addAutoReleasedObject( new DataprocessorFactory( this, mTypes ) );
#endif
    } while ( 0 );

    if ( ! impl_->init_mode() )
        return false;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    addAutoReleasedObject( new NavigationWidgetFactory );
#else
    ExtensionSystem::PluginManager::addObject( new NavigationWidgetFactory );
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect( Core::ICore::instance(), &Core::ICore::coreAboutToOpen, this, [](){
            Core::NavigationWidget::instance()->activateSubWidget( Constants::C_DATAPROC_NAVI ); });
#else
    connect( Core::ICore::instance(), &Core::ICore::coreAboutToOpen, this, [](){
        Core::NavigationWidget::activateSubWidget( Constants::C_DATAPROC_NAVI, Core::Side::Left );
    });
#endif
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

    if ( adextension::iSessionManager * mgr = SessionManager::instance() ) {
        ExtensionSystem::PluginManager::removeObject( mgr );
        delete mgr;
    }

#if ! defined NDEBUG
    ADDEBUG() << "## Shutdown: "
              << "\t" << boost::filesystem::relative( boost::dll::this_line_location()
                                                     , boost::dll::program_location().parent_path() );
#endif

	return SynchronousShutdown;
}

// bool
// DataprocPlugin::connect_isnapshothandler_signals()
// {
//     connect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onPortfolioCreated, this, [] ( const QString& _1 ) {
//             document::instance()->handle_portfolio_created( _1 );
//         } );

//     connect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onFoliumAdded, this, [] ( const QString& _1, const QString& _2, const QString& _3 ) {
//             document::instance()->handle_folium_added( _1, _2, _3 );
//         } );

//     return true;
// }

// void
// DataprocPlugin::disconnect_isnapshothandler_signals()
// {
//     //disconnect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onPortfolioCreated, this, &dataproc_document::handle_portfolio_created );
//     //disconnect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onFoliumAdded, this, &dataproc_document::handle_folium_added );
// }

//////////////


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_EXPORT_PLUGIN( DataprocPlugin )
#endif
