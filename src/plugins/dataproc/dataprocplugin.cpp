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

#include "dataprocplugin.hpp"
#include "actionmanager.hpp"
#include "constants.hpp"
#include "mode.hpp"
#include "editorfactory.hpp"
#include "dataprocessor.hpp"
#include "dataprocessorfactory.hpp"
#include "dataproceditor.hpp"
#include "isequenceimpl.hpp"
#include "isnapshothandlerimpl.hpp"
#include "ipeptidehandlerimpl.hpp"
#include "mainwindow.hpp"
#include "navigationwidgetfactory.hpp"
#include "sessionmanager.hpp"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <acewrapper/constants.hpp>
#include <adcontrols/datafilebroker.hpp>
#include <adcontrols/datafile_factory.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <adextension/ieditorfactory.hpp>
#include <adextension/isnapshothandler.hpp>

#include <adplugin/plugin.hpp>
#include <adplugin/loader.hpp>
#include <adplugin/widget_factory.hpp>

#include <adplugin/constants.hpp>
#include <adplugin/manager.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/configuration.hpp>
#include <adportable/configloader.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug_core.hpp>
#include <adlog/logger.hpp>
#include <adlog/logging_handler.hpp>
#include <adportable/float.hpp>
#include <adwidgets/centroidform.hpp>
#include <portfolio/logging_hook.hpp>
#include <portfolio/folium.hpp>
#include <qtwrapper/qstring.hpp>
#include <qtwrapper/application.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <xmlparser/pugixml.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
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

namespace dataproc {
    class mimeTypeHelper {
    public:
        static bool add( Core::MimeDatabase * mdb, const char * xml, int len, QString*& emsg ) {
            if ( mdb && xml ) {
                QBuffer io;
                io.setData( xml, len );
                io.open( QIODevice::ReadOnly );
                return  mdb->addMimeTypes( &io, emsg );
            }
            return false;
        }
        static bool populate( QStringList& vec, const char * xml ) {
            pugi::xml_document doc;
            if ( doc.load( xml ) ) {
                pugi::xpath_node_set list = doc.select_nodes( "/mime-info/mime-type" );
                for ( auto it = list.begin(); it != list.end(); ++it )
                    vec << it->node().attribute( "type" ).value();
				return true;
			}
			return false;
        }
    };
}

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
                                 , dataprocFactory_( 0 )
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

    QList<int> context;
    if ( Core::UniqueIDManager * uidm = core->uniqueIDManager() ) {
        context.append( uidm->uniqueIdentifier( Constants::C_DATAPROCESSOR ) );
    }

    //-------------------------------------------------------------------------------------------
    const wchar_t * query = L"/DataprocConfiguration/Configuration";
    std::wstring apppath = qtwrapper::application::path( L".." ); // := "~/qtplatz/bin/.."
    std::wstring configFile = adplugin::loader::config_fullpath( apppath, L"/MS-Cheminformatics/dataproc.config" );
    boost::filesystem::path plugindir = boost::filesystem::path( configFile ).branch_path();
    
	adplugin::loader::populate( plugindir.generic_wstring().c_str() );

    pConfig_.reset( new adportable::Configuration() );
    adportable::Configuration& config = *pConfig_;

    if ( ! adportable::ConfigLoader::loadConfigFile( config, configFile, query ) ) {
        *error_message = "loadConfig load failed";
        return false;
    }
    //------------------------------------------------

    Core::MimeDatabase* mdb = core->mimeDatabase();
    if ( ! mdb ) {
        *error_message = "no mime database in Core plugin";
        return false;
    }

    do {
        std::vector< std::string > mime;
        std::vector< adplugin::plugin_ptr > dataproviders;
        if ( adplugin::loader::select_iids( ".*\\.adplugins\\.datafile_factory\\..*", dataproviders ) ) {
            
            std::for_each( dataproviders.begin(), dataproviders.end(), [&]( const adplugin::plugin_ptr& d ){
                    adcontrols::datafile_factory * factory = d->query_interface< adcontrols::datafile_factory >();
                    if ( factory ) {
                        adcontrols::datafileBroker::register_factory( factory, factory->name() );
                        if ( factory->mimeTypes() )
                            mime.push_back( factory->mimeTypes() );
                    }
                });
        }
        
        //------------------------------------------------
        QStringList mTypes;
        
        // externally installed mime-types
        std::wstring mimefile
            = adplugin::loader::config_fullpath( apppath, L"/MS-Cheminformatics/dataproc-mimetype.xml" );
        if ( mdb->addMimeTypes( qtwrapper::qstring( mimefile ), error_message) ) {
            pugi::xml_document doc;
            if ( doc.load_file( mimefile.c_str() ) ) {
                pugi::xpath_node_set list = doc.select_nodes( "/mime-info/mime-type" );
                for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it )
                    mTypes << it->node().attribute( "type" ).value();
            }
        }

        std::for_each( mime.begin(), mime.end(), [&](const std::string& xml){
                if ( mimeTypeHelper::add( mdb, xml.c_str(), static_cast<int>(xml.size()), error_message ) )
					mimeTypeHelper::populate( mTypes, xml.c_str() );
            });


        // core mime-types
        if ( ! mdb->addMimeTypes(":/dataproc/mimetype.xml", error_message) )
            ADWARN() << "addMimeTypes" << ":/dataproc/mimetype.xml" << error_message;


        dataprocFactory_ = new DataprocessorFactory( this, mTypes );
        addAutoReleasedObject( dataprocFactory_ );
    } while ( 0 );

    mode_.reset( new dataproc::Mode( this ) );
    if ( ! mode_ )
        return false;
    mode_->setContext( context );

    pActionManager_->initialize_actions( context );
    mainWindow_->activateLayout();
    QWidget * widget = mainWindow_->createContents( mode_.get(), config, apppath );
    mode_->setWidget( widget );

    iSequence_.reset( new iSequenceImpl );
    if ( iSequence_ && mainWindow_->editor_factories( *iSequence_ ) )
        addObject( iSequence_.get() );

    iSnapshotHandler_.reset( new iSnapshotHandlerImpl );
    if ( iSnapshotHandler_ && connect_isnapshothandler_signals() )
        addObject( iSnapshotHandler_.get() );

    iPeptideHandler_.reset( new iPeptideHandlerImpl );
	if ( iPeptideHandler_ )
        addObject( iPeptideHandler_.get() );

    addObject( mode_.get() );
    addAutoReleasedObject( new NavigationWidgetFactory );

    return true;
}

void
DataprocPlugin::applyMethod( const adcontrols::ProcessMethod& m )
{
	emit onApplyMethod( m );
}

void
DataprocPlugin::handleFileCreated( const QString& filename )
{
    handle_portfolio_created( filename );
}

void
DataprocPlugin::handle_portfolio_created( const QString filename )
{
    // simulate file->open()
    Core::ICore * core = Core::ICore::instance();
    if ( core ) {
        Core::EditorManager * em = core->editorManager();
        if ( em && dataprocFactory_ ) {
            if ( Core::IEditor * ie = dataprocFactory_->createEditor( 0 ) ) {
                if ( DataprocEditor * editor = dynamic_cast< DataprocEditor * >( ie ) ) {
                    editor->portfolio_create( filename );
                    em->pushEditor( editor );
                }
            }
        }
    }
}

void
DataprocPlugin::handle_folium_added( const QString fname, const QString path, const QString id )
{
    qDebug() << "===== DataprocPlugin::handle_folium_added" << fname << " path=" << path;

	std::wstring filename = fname.toStdWString();

    SessionManager::vector_type::iterator it = SessionManager::instance()->find( filename );
    if ( it == SessionManager::instance()->end() ) {
		Core::EditorManager::instance()->openEditor( fname );
        it = SessionManager::instance()->find( filename );
    }
    if ( it != SessionManager::instance()->end() ) {
		Dataprocessor& processor = it->getDataprocessor();
		processor.load( path.toStdWString(), id.toStdWString() );
    }
}

void
DataprocPlugin::onSelectSpectrum( double /*minutes*/, size_t pos, int fcn )
{
	qtwrapper::waitCursor w;

	Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor();
	if ( dp ) {
		if ( const adcontrols::LCMSDataset * dset = dp->getLCMSDataset() ) {
			adcontrols::MassSpectrum ms;
            try {
                std::wostringstream text;
                // size_t pos = dset->find_scan( index, fcn );
                if ( dset->getSpectrum( fcn, pos, ms ) ) {
                    double t = dset->timeFromPos( pos ) / 60.0;
                    if ( !adportable::compare<double>::approximatelyEqual( ms.getMSProperty().timeSinceInjection(), 0.0 ) )
                        t = ms.getMSProperty().timeSinceInjection() / 60.0; // to min
                    text << boost::wformat( L"Spectrum #%d fcn:%d/%d @ %.3lfmin" ) % pos % ms.protocolId() % ms.nProtocols() % t;
                    adcontrols::ProcessMethod m;
                    ms.addDescription( adcontrols::Description( L"create", text.str() ) );
                    portfolio::Folium folium = dp->addSpectrum( ms, m );
                }
            }
            catch ( ... ) {
                QMessageBox::warning( 0, "DataprocPlugin", boost::current_exception_diagnostic_information().c_str() );
            }
        }
    }
}

void
DataprocPlugin::onSelectTimeRangeOnChromatogram( double x1, double x2 )
{
	qtwrapper::waitCursor w;

    ADTRACE() << "onSelectTimeRagneOnChromatogram(" << x1 << ", " << x2 << ")";

	Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor();
	if ( dp ) {
		if ( const adcontrols::LCMSDataset * dset = dp->getLCMSDataset() ) {
            size_t pos1 = dset->posFromTime( adcontrols::Chromatogram::toSeconds( x1 ) ); // min --> sec
            size_t pos2 = dset->posFromTime( adcontrols::Chromatogram::toSeconds( x2 ) ); // min --> sec
            int pos = static_cast<int>(pos1);
            double t1 = adcontrols::Chromatogram::toMinutes( dset->timeFromPos( pos1 ) ); // to minuites
            double t2 = adcontrols::Chromatogram::toMinutes( dset->timeFromPos( pos2 ) ); // to minutes

			adcontrols::MassSpectrum ms;
            try {
                if ( dset->getSpectrum( -1, pos++, ms ) ) {
                    if ( !adportable::compare<double>::approximatelyEqual( ms.getMSProperty().timeSinceInjection(), 0.0 ) )
                        t1 = ms.getMSProperty().timeSinceInjection() / 60.0; // to min
                
                    std::wostringstream text;
                    if ( pos2 > pos1 ) {
                        QProgressBar progressBar;
                        progressBar.setRange( static_cast<int>(pos1), static_cast<int>(pos2) );
                        progressBar.setVisible( true );
                    
                        adcontrols::MassSpectrum a;
                        while ( pos < int(pos2)	&& dset->getSpectrum( 0, pos++, a ) ) {
                            progressBar.setValue( pos );
                            ms += a;
                        }
                        if ( !adportable::compare<double>::approximatelyEqual( a.getMSProperty().timeSinceInjection(), 0.0 ) )
                            t2 = a.getMSProperty().timeSinceInjection() / 60.0; // to min
                        text << L"Spectrum (" << std::fixed << std::setprecision(3) << t1 << " - " << t2 << ")min";
                    } else {
                        text << L"Spectrum @ " << std::fixed << std::setprecision(3) << t1 << "min";
                    }
                    adcontrols::ProcessMethod m;
                    ms.addDescription( adcontrols::Description( L"create", text.str() ) );
                    portfolio::Folium folium = dp->addSpectrum( ms, m );
                
                    // add centroid spectrum if exist (Bruker's compassXtract returns centroid as 2nd function)
                    if ( folium ) {
                        bool hasCentroid( false );
                        if ( pos1 == pos2 && dset->hasProcessedSpectrum( 0, static_cast<int>(pos1) ) ) {
                            adcontrols::MassSpectrumPtr pCentroid( new adcontrols::MassSpectrum );
                            if ( dset->getSpectrum( 0, static_cast<int>(pos1), *pCentroid, dset->findObjId( L"MS.CENTROID" ) ) ) {
                                hasCentroid = true;
                                portfolio::Folium att = folium.addAttachment( L"Centroid Spectrum" );
                                att.assign( pCentroid, pCentroid->dataClass() );
                                SessionManager::instance()->updateDataprocessor( dp, folium );
                            }
                        }
                        if ( ! hasCentroid ) {
                            mainWindow_->getProcessMethod( m );
                            //dp->applyProcess( folium, m, CentroidProcess );
                        }
                    }
                }
            } catch ( ... ) {
                ADTRACE() << boost::current_exception_diagnostic_information();
                QMessageBox::warning( 0, "DataprocPlugin", boost::current_exception_diagnostic_information().c_str() );
            }
		}
	}
    ADTRACE() << "return onSelectTimeRagneOnChromatogram(" << x1 << ", " << x2 << ")";
}

void
DataprocPlugin::extensionsInitialized()
{
    mainWindow_->OnInitialUpdate();
    pActionManager_->loadDefaults();
}

ExtensionSystem::IPlugin::ShutdownFlag
DataprocPlugin::aboutToShutdown()
{
    ADTRACE() << "====== DataprocPlugin shutting down...  ===============";

    pActionManager_->saveDefaults();

    mainWindow_->OnFinalClose();

    ADTRACE() << "====== DataprocPlugin shutdown complete ===============";
	return SynchronousShutdown;
}

// bool
// DataprocPlugin::install_isequence( const adportable::Configuration& config
//                                    , const std::wstring& , iSequenceImpl& impl )
// {
//     impl << detail::iEditorFactoryImpl( [] ( QWidget * p )->QWidget*{ return new adwidgets::CentroidForm( p ); }
//         , adextension::iEditorFactory::PROCESS_METHOD
//         , "Centroid" );

//     impl << detail::iEditorFactoryImpl( [] ( QWidget * p )->QWidget*{ return new adwidgets::TargetingWidget( p ); }
//         , adextension::iEditorFactory::PROCESS_METHOD
//         , "Targeting" );

//     // using adportable::Configuration;
//     // const Configuration * tab = Configuration::find( config, "ProcessMethodEditors" );    
//     // if ( tab ) {
//     //     for ( auto it : *tab ) {
//     //         // std::for_each( tab->begin(), tab->end(), [&] ( Configuration::vector_type::const_iterator it ){
//     //         const std::string& iid = it.component_interface();
//     //         impl << detail::iEditorFactoryImpl( [=] ( QWidget * p )->QWidget*{ return adplugin::widget_factory::create( iid.c_str(), 0, p ); }
//     //             , adextension::iEditorFactory::PROCESS_METHOD
//     //             , QString::fromStdWString( it.title() ) );
//     //     }
//     // }

//     return impl.size();
// }

void
DataprocPlugin::delete_editorfactories( std::vector< EditorFactory * >& factories )
{
    for ( size_t i = 0; i < factories.size(); ++i )
        delete factories[ i ];
    factories.clear();
}

bool
DataprocPlugin::connect_isnapshothandler_signals()
{
    connect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onPortfolioCreated, this, &DataprocPlugin::handle_portfolio_created );
    connect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onFoliumAdded, this, &DataprocPlugin::handle_folium_added );

    return true;
}

void
DataprocPlugin::disconnect_isnapshothandler_signals()
{
    disconnect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onPortfolioCreated, this, &DataprocPlugin::handle_portfolio_created );
    disconnect( iSnapshotHandler_.get(), &iSnapshotHandlerImpl::onFoliumAdded, this, &DataprocPlugin::handle_folium_added );
}

Q_EXPORT_PLUGIN( DataprocPlugin )
