// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include "dataproc_document.hpp"
#include "dataprocessor.hpp"
#include "dataprocessorfactory.hpp"
#include "dataproceditor.hpp"
#include <adextension/isequenceimpl.hpp>
#include "isnapshothandlerimpl.hpp"
#include "ipeptidehandlerimpl.hpp"
#include "mainwindow.hpp"
#include "mimetypehelper.hpp"
#include "mode.hpp"
#include "navigationwidgetfactory.hpp"
#include "sessionmanager.hpp"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
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

#include <adplugin/plugin.hpp>
#include <adplugin/constants.hpp>
#include <adplugin_manager/loader.hpp>
#include <adplugin_manager/manager.hpp>

#include <adportable/array_wrapper.hpp>
#include <adportable/configuration.hpp>
#include <adportable/configloader.hpp>
#include <adportable/debug.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug_core.hpp>
#include <adlog/logger.hpp>
#include <adlog/logging_handler.hpp>
#include <adportable/float.hpp>
#include <adwidgets/centroidform.hpp>
#include <adwidgets/progresswnd.hpp>
#include <adportfolio/logging_hook.hpp>
#include <adportfolio/folium.hpp>
#include <qtwrapper/qstring.hpp>
#include <qtwrapper/application.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <xmlparser/pugixml.hpp>

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

#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/exception/all.hpp>
#include <streambuf>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <thread>

using namespace dataproc;

DataprocPlugin * DataprocPlugin::instance_ = 0;

DataprocPlugin::~DataprocPlugin()
{
    if ( mode_ )
        removeObject( mode_.get() );

    if ( iSequence_ )
        removeObject( iSequence_.get() );

    if ( iSnapshotHandler_ ) {
        disconnect_isnapshothandler_signals();
        removeObject( iSnapshotHandler_.get() );
    }

    if ( iPeptideHandler_ )
        removeObject( iPeptideHandler_.get() );
}

DataprocPlugin::DataprocPlugin() : mainWindow_( new MainWindow )
                                 , pSessionManager_( new SessionManager() )
                                 , pActionManager_( new ActionManager( this ) ) 
{
    instance_ = this;
}

bool
DataprocPlugin::initialize( const QStringList& arguments, QString* error_message )
{
    Q_UNUSED( arguments );

    do {
        adportable::core::debug_core::instance()->hook( adlog::logging_handler::log );
        portfolio::logging_hook::register_hook( adlog::logging_handler::log );
    } while(0);

    Core::ICore * core = Core::ICore::instance();
    if ( core == 0 )
        return false;

    // Core::Context context( (Core::Id( Constants::C_DATAPROCESSOR )) );

    //-------------------------------------------------------------------------------------------
    const wchar_t * query = L"/DataprocConfiguration/Configuration";
    std::wstring apppath = qtwrapper::application::path( L".." ); // := "~/qtplatz/bin/.."
    std::wstring configFile = adplugin::loader::config_fullpath( apppath, L"/MS-Cheminformatics/dataproc.config" );
    //boost::filesystem::path plugindir = boost::filesystem::path( configFile ).branch_path();
    
    try {
        adplugin::loader::populate( apppath.c_str() );
    } catch ( boost::exception& ex ) {
        QMessageBox::warning( 0, tr( "Processing" ), boost::diagnostic_information( ex ).c_str() );
    }

    pConfig_.reset( new adportable::Configuration() );
    adportable::Configuration& config = *pConfig_;

    if ( ! adportable::ConfigLoader::loadConfigFile( config, configFile, query ) ) {
        *error_message = "loadConfig load failed";
        return false;
    }
    //------------------------------------------------

    do {
        std::vector< std::string > mime;
        do {
            std::vector< adplugin::plugin_ptr > dataproviders;
            if ( adplugin::loader::select_iids( ".*\\.adplugins\\.datafile_factory\\..*", dataproviders ) ) {

                std::for_each( dataproviders.begin(), dataproviders.end(), [&] ( const adplugin::plugin_ptr& d ) {
                    adcontrols::datafile_factory * factory = d->query_interface< adcontrols::datafile_factory >();
                    if ( factory ) {
                        adcontrols::datafileBroker::register_factory( factory, d->clsid() ); // factory->name() );
                        if ( factory->mimeTypes() )
                            mime.push_back( factory->mimeTypes() );
                    }
                } );
                long x = 0;
            }
        } while ( 0 );
        
        //------------------------------------------------
        QStringList mTypes;
        
        // externally installed mime-types
        std::wstring mimefile = adplugin::loader::config_fullpath( apppath, L"/MS-Cheminformatics/dataproc-mimetype.xml" );

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


        // core mime-types
        if ( !Core::MimeDatabase::addMimeTypes( ":/dataproc/mimetype.xml", error_message ) )
            ADWARN() << "addMimeTypes" << ":/dataproc/mimetype.xml" << error_message;

        dataproc_document::instance()->setDataprocessorFactory( std::make_unique< DataprocessorFactory >( this, mTypes ) );
        
        addAutoReleasedObject( dataproc_document::instance()->dataprocessorFactory() );
        
    } while ( 0 );

    mode_.reset( new dataproc::Mode( this ) );
    if ( ! mode_ )
        return false;
    // mode_->setContext( context );

    pActionManager_->initialize_actions( mode_->context() );
    mainWindow_->activateLayout();
    QWidget * widget = mainWindow_->createContents( mode_.get() );
    widget->setObjectName( QLatin1String( "DataprocessingPage") );
    mode_->setWidget( widget );

    // iSequence_.reset( new iSequenceImpl );
    // if ( iSequence_ && mainWindow_->editor_factories( *iSequence_ ) )
    //     addObject( iSequence_.get() );

    iSnapshotHandler_.reset( new iSnapshotHandlerImpl );
    if ( iSnapshotHandler_ && connect_isnapshothandler_signals() )
        addObject( iSnapshotHandler_.get() );

    iPeptideHandler_.reset( new iPeptideHandlerImpl );
	if ( iPeptideHandler_ )
        addObject( iPeptideHandler_.get() );

    addObject( mode_.get() );
    addAutoReleasedObject( new NavigationWidgetFactory );

    connect( Core::ICore::instance(), &Core::ICore::coreAboutToOpen, this, [](){
            Core::NavigationWidget::instance()->activateSubWidget( Constants::C_DATAPROC_NAVI ); });

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
    mainWindow_->OnInitialUpdate();
    dataproc_document::instance()->initialSetup();
}


ExtensionSystem::IPlugin::ShutdownFlag
DataprocPlugin::aboutToShutdown()
{
    ADTRACE() << "====== DataprocPlugin shutting down...  ===============";

    dataproc_document::instance()->finalClose();

    mainWindow_->OnFinalClose();

    ADTRACE() << "====== DataprocPlugin shutdown complete ===============";
	return SynchronousShutdown;
}

bool
DataprocPlugin::connect_isnapshothandler_signals()
{
    connect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onPortfolioCreated, this, [] ( const QString& _1 ) {
            dataproc_document::instance()->handle_portfolio_created( _1 );
        } );

    connect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onFoliumAdded, this, [] ( const QString& _1, const QString& _2, const QString& _3 ) {
            dataproc_document::instance()->handle_folium_added( _1, _2, _3 );
        } );

    return true;
}

void
DataprocPlugin::disconnect_isnapshothandler_signals()
{
    //disconnect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onPortfolioCreated, this, &dataproc_document::handle_portfolio_created );
    //disconnect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onFoliumAdded, this, &dataproc_document::handle_folium_added );
}

Q_EXPORT_PLUGIN( DataprocPlugin )
