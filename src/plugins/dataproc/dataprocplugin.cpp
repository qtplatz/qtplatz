// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include "dataprocmanager.hpp"
#include "editorfactory.hpp"
#include "dataprocessor.hpp"
#include "dataprocessorfactory.hpp"
#include "dataproceditor.hpp"
#include "isequenceimpl.hpp"
#include "mainwindow.hpp"
#include "navigationwidgetfactory.hpp"
#include "sessionmanager.hpp"

#include <acewrapper/brokerhelper.hpp>
#include <acewrapper/constants.hpp>
#include <acewrapper/input_buffer.hpp>
#include <adcontrols/datafilebroker.hpp>
#include <adcontrols/datafile_factory.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <adextension/ieditorfactory.hpp>

//#include <adplugin/adplugin.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/loader.hpp>
//#include <adplugin/orbLoader.hpp>
#include <adplugin/widget_factory.hpp>

#include <adplugin/constants.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/orbmanager.hpp>
#include <adplugin/qbrokersessionevent.hpp>
#include <adportable/configuration.hpp>
#include <adportable/configloader.hpp>
#include <adportable/debug.hpp>
#include <portfolio/folium.hpp>
#include <qtwrapper/qstring.hpp>
#include <qtwrapper/application.hpp>
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
#include <boost/foreach.hpp>
#include <streambuf>
#include <fstream>
#include <iomanip>

#include <adinterface/brokerC.h>

using namespace dataproc;

DataprocPlugin * DataprocPlugin::instance_ = 0;

DataprocPlugin::~DataprocPlugin()
{
    if ( mode_ )
        removeObject( mode_.get() );

    if ( iSequence_ )
        removeObject( iSequence_.get() );
}

DataprocPlugin::DataprocPlugin() : mainWindow_( new MainWindow )
                                 , pSessionManager_( new SessionManager() )
                                 , pActionManager_( new ActionManager( this ) ) 
                                 , pBrokerSessionEvent_( 0 )
                                 , brokerSession_( 0 ) 
                                 , dataprocFactory_( 0 )
{
    instance_ = this;
    ACE::init();
}

bool
DataprocPlugin::initialize( const QStringList& arguments, QString* error_message )
{
    Q_UNUSED( arguments );

    Core::ICore * core = Core::ICore::instance();
  
    QList<int> context;
    if ( core ) {
        Core::UniqueIDManager * uidm = core->uniqueIDManager();
        if ( uidm ) {
            context.append( uidm->uniqueIdentifier( Constants::C_DATAPROCESSOR ) );
            context.append( uidm->uniqueIdentifier( Core::Constants::C_EDITORMANAGER ) );
            context.append( uidm->uniqueIdentifier( Core::Constants::C_NAVIGATION_PANE ) );
        }
    } else
        return false;

    //-------------------------------------------------------------------------------------------
    const wchar_t * query = L"/DataprocConfiguration/Configuration";
    std::wstring apppath = qtwrapper::application::path( L".." ); // := "~/qtplatz/bin/.."
    std::wstring configFile = adplugin::loader::config_fullpath( apppath, L"/MS-Cheminformatics/dataproc.config.xml" );
    boost::filesystem::path plugindir = boost::filesystem::path( configFile ).branch_path();
    
	adplugin::loader::populate( plugindir.generic_wstring().c_str() );

    pConfig_.reset( new adportable::Configuration() );
    adportable::Configuration& config = *pConfig_;

    if ( ! adportable::ConfigLoader::loadConfigFile( config, configFile, query ) ) {
        *error_message = "loadConfig load failed";
        return false;
    }
    //------------------------------------------------

    // dataprovider installation move to servantplugin
    // install_dataprovider( config, apppath );
	std::vector< adplugin::plugin_ptr > dataproviders;
	if ( adplugin::loader::select_iids( ".*\\.datafile_factory$", dataproviders ) ) {
		BOOST_FOREACH( const adplugin::plugin_ptr& d, dataproviders ) {
			adcontrols::datafile_factory * factory = d->query_interface< adcontrols::datafile_factory >();
			if ( factory ) 
				adcontrols::datafileBroker::register_factory( factory, factory->name() );
		}
	}

    iSequence_.reset( new iSequenceImpl );
    if ( iSequence_ && install_isequence( config, apppath, *iSequence_ ) ) {
        addObject( iSequence_.get() );
    }

    //------------------------------------------------

    // DataprocessorFactory * dataprocFactory = 0;
    Core::MimeDatabase* mdb = core->mimeDatabase();
    if ( mdb ) {
        // externally installed mime-types
        std::wstring mimefile
            = adplugin::loader::config_fullpath( apppath, L"/MS-Cheminformatics/dataproc-mimetype.xml" );
        if ( ! mdb->addMimeTypes( qtwrapper::qstring( mimefile ), error_message) )
            adportable::debug( __FILE__, __LINE__ ) << "addMimeTypes" << mimefile << error_message;

        // core mime-types
        if ( ! mdb->addMimeTypes(":/dataproc/mimetype.xml", error_message) )
            adportable::debug( __FILE__, __LINE__ ) << "addMimeTypes" << ":/dataproc/mimetype.xml" << error_message;

        QStringList mTypes;
        pugi::xml_document doc;
        if ( doc.load_file( mimefile.c_str() ) ) {
            pugi::xpath_node_set list = doc.select_nodes( "/mime-info/mime-type" );
            for ( pugi::xpath_node_set::const_iterator it = list.begin(); it != list.end(); ++it )
                mTypes << it->node().attribute( "type" ).value();
        }
        dataprocFactory_ = new DataprocessorFactory( this, mTypes );
        addAutoReleasedObject( dataprocFactory_ );
    }

    mode_.reset( new dataproc::Mode( this ) );
    if ( ! mode_ )
        return false;
    mode_->setContext( context );

    pActionManager_->initialize_actions( context );
    mainWindow_->activateLayout();
    mainWindow_->createActions();
    QWidget * widget = mainWindow_->createContents( mode_.get(), config, apppath );
    mode_->setWidget( widget );

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
DataprocPlugin::handle_portfolio_created( const QString token )
{
    // simulate file->open()
    Core::ICore * core = Core::ICore::instance();
    if ( core ) {
        Core::EditorManager * em = core->editorManager();
        if ( em && dataprocFactory_ ) {
            Core::IEditor * ie = dataprocFactory_->createEditor( 0 );
            DataprocEditor * editor = dynamic_cast< DataprocEditor * >( ie );
            if ( editor ) {
                editor->portfolio_create( token );
                em->pushEditor( editor );
            }
        }
    }
}

void
DataprocPlugin::handle_folium_added( const QString token, const QString path, const QString id )
{
    qDebug() << "===== DataprocPlugin::handle_folium_added" << token << " path=" << path;

    SessionManager::vector_type::iterator it = SessionManager::instance()->find( qtwrapper::wstring( token ) );
    if ( it == SessionManager::instance()->end() ) {
        boost::filesystem::path xtoken( qtwrapper::wstring::copy( token ) );
        xtoken.replace_extension( L".adfs" );
        it = SessionManager::instance()->find( xtoken.wstring() );
    }
    if ( it != SessionManager::instance()->end() ) {
        
        Broker::Folium_var var = brokerSession_->folium( qtwrapper::wstring( token ).c_str(), qtwrapper::wstring( id ).c_str() );

        // todo check type
        acewrapper::input_buffer ibuffer( var->serialized.get_buffer(), var->serialized.length() );
        std::istream in( &ibuffer );

        adcontrols::MassSpectrum ms;
        adcontrols::MassSpectrum::restore( in, ms );

        Dataprocessor& processor = it->getDataprocessor();

        adcontrols::ProcessMethod m;
        processor.addSpectrum( ms, m );
    }
}

void
DataprocPlugin::onSelectTimeRangeOnChromatogram( double x1, double x2 )
{
	Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor();
	if ( brokerSession_ && dp ) {
#if 0
		// TODO:  observer access has object delete twince, that will cause debug assertion failuer
		// SignalObserver::Observer_var observer = dp->observer();
		brokerSession_->coaddSpectrumEx( token.c_str(), observer, x, x );
#endif
		const adcontrols::LCMSDataset * dset = dp->getLCMSDataset();
		if ( dset ) {
			long pos1 = dset->posFromTime( x1 );
			long pos2 = dset->posFromTime( x2 );
			long pos = pos1;

			adcontrols::MassSpectrum ms;
			if ( dset->getSpectrum( 0, pos++, ms ) ) {
				double t1 = ms.getMSProperty().timeSinceInjection() / 60.0e6; // usec -> min
				std::wostringstream text;
				if ( pos2 > pos1 ) {
                    QProgressBar progressBar;
                    progressBar.setRange( pos1, pos2 );
                    progressBar.setVisible( true );

					adcontrols::MassSpectrum a;
					while ( pos1 < pos2	&& dset->getSpectrum( 0, pos++, a ) ) {
						progressBar.setValue( pos );
						ms += a;
					}
					double t2 = a.getMSProperty().timeSinceInjection() / 60.0e6; // usec -> min
					text << L"Spectrum (" << std::fixed << std::setprecision(3) << t1 << " - " << t2 << ")";
				} else {
					text << L"Spectrum @ " << std::fixed << std::setprecision(3) << t1 << "min";
				}
				adcontrols::ProcessMethod m;
				ms.addDescription( adcontrols::Description( L"create", text.str() ) );
				portfolio::Folium folium = dp->addSpectrum( ms, m );

				// add centroid spectrum if exist
				if ( folium && pos1 == pos2 && dset->getFunctionCount() >= 2 ) {
					adcontrols::MassSpectrumPtr pCentroid( new adcontrols::MassSpectrum );
					if ( dset->getSpectrum( 1, pos1, *pCentroid ) ) {
						portfolio::Folium att = folium.addAttachment( L"Centroid Spectrum" );
                        att.assign( pCentroid, pCentroid->dataClass() );

						SessionManager::instance()->updateDataprocessor( dp, folium );
					}
				}
			}
		}
	}
}

void
DataprocPlugin::extensionsInitialized()
{
    do {
        std::string ior = adplugin::manager::iorBroker();
        if ( ! ior.empty() ) {
            CORBA::ORB_var orb = adplugin::ORBManager::instance()->orb();
            Broker::Manager_var mgr = acewrapper::brokerhelper::getManager( orb, ior );
            if ( ! CORBA::is_nil( mgr ) ) {
                brokerSession_ = mgr->getSession( L"acquire" );
                pBrokerSessionEvent_ = new QBrokerSessionEvent;
                brokerSession_->connect( "-user-", "-password-", "dataproc", pBrokerSessionEvent_->_this() );
                connect( pBrokerSessionEvent_, SIGNAL( signal_portfolio_created( const QString ) )
                         , this, SLOT(handle_portfolio_created( const QString )) );
                connect( pBrokerSessionEvent_, SIGNAL( signal_folium_added( const QString, const QString, const QString ) )
                         , this, SLOT(handle_folium_added( const QString, const QString, const QString )) );
            }
        } else {
            QMessageBox::critical( 0, "DataprocPlugin::extensionsInitialized"
                                   , "can't find ior for adbroker -- maybe servant plugin load failed.");
        }
    } while(0);
	mainWindow_->OnInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
DataprocPlugin::aboutToShutdown()
{
    adportable::debug(__FILE__, __LINE__) << "====== DataprocPlugin shutting down...  ===============";

    mainWindow_->OnFinalClose();

	if ( ! CORBA::is_nil( brokerSession_ ) ) {

        disconnect( pBrokerSessionEvent_, SIGNAL( signal_portfolio_created( const QString ) )
                    , this, SLOT(handle_portfolio_created( const QString )) );

        disconnect( pBrokerSessionEvent_, SIGNAL( signal_folium_added( const QString, const QString, const QString ) )
                    , this, SLOT(handle_folium_added( const QString, const QString, const QString )) );

        brokerSession_->disconnect( pBrokerSessionEvent_->_this() );

        // destruct event sink object -->
        CORBA::release( pBrokerSessionEvent_->_this() ); // delete object reference
        adplugin::ORBManager::instance()->deactivate( pBrokerSessionEvent_ );

        delete pBrokerSessionEvent_;
        pBrokerSessionEvent_ = 0;
        // <-- end event sink desctruction

        CORBA::release( brokerSession_ );
        brokerSession_ = 0;
    }
    adportable::debug(__FILE__, __LINE__) << "====== DataprocPlugin shutdown complete ===============";
	return SynchronousShutdown;
}

// static
// bool
// DataprocPlugin::install_dataprovider( const adportable::Configuration& config, const std::wstring& apppath )
// {
//     const adportable::Configuration * provider = adportable::Configuration::find( config, L"dataproviders" );
//     if ( provider ) {
//         for ( adportable::Configuration::vector_type::const_iterator it = provider->begin(); it != provider->end(); ++it ) {
//             const std::wstring name = adplugin::orbLoader::library_fullpath( apppath, it->module().library_filename() );
//             adcontrols::datafileBroker::register_library( name );
//         }
// 		return true;
//     }
// 	return false;
// }

bool
DataprocPlugin::install_isequence( const adportable::Configuration& config
                                   , const std::wstring& apppath
                                   , iSequenceImpl& impl )
{
    using adportable::Configuration;
    const Configuration * tab = Configuration::find( config, L"ProcessMethodEditors" );    
    if ( tab ) {
        for ( Configuration::vector_type::const_iterator it = tab->begin(); it != tab->end(); ++it )
			impl << iEditorFactoryPtr( new EditorFactory( *it, apppath ) );
    }
	return impl.size();
}

void
DataprocPlugin::delete_editorfactories( std::vector< EditorFactory * >& factories )
{
    for ( size_t i = 0; i < factories.size(); ++i )
        delete factories[ i ];
    factories.clear();
}

Q_EXPORT_PLUGIN( DataprocPlugin )
