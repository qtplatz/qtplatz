// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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
#include "editorfactory.hpp"
#include "dataprocessor.hpp"
#include "dataprocessorfactory.hpp"
#include "dataproceditor.hpp"
#include "isequenceimpl.hpp"
#include "mainwindow.hpp"
#include "navigationwidgetfactory.hpp"
#include "sessionmanager.hpp"
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <acewrapper/constants.hpp>
#include <acewrapper/input_buffer.hpp>
#include <adcontrols/datafilebroker.hpp>
#include <adcontrols/datafile_factory.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <adextension/ieditorfactory.hpp>
#include <adorbmgr/orbmgr.hpp>

#include <adplugin/plugin.hpp>
#include <adplugin/loader.hpp>
#include <adplugin/widget_factory.hpp>

#include <adplugin/constants.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/qbrokersessionevent.hpp>
#include <adportable/array_wrapper.hpp>
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
#include <streambuf>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <adinterface/brokerC.h>

namespace dataproc {
    class mimeTypeHelper {
    public:
        static bool add( Core::MimeDatabase * mdb, const char * xml, size_t len, QString*& emsg ) {
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

	class waitCursor {
	public:
		waitCursor() {
			QApplication::setOverrideCursor( Qt::WaitCursor );
		}
		~waitCursor() {
			QApplication::restoreOverrideCursor();
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
}

DataprocPlugin::DataprocPlugin() : mainWindow_( new MainWindow )
                                 , pSessionManager_( new SessionManager() )
                                 , pActionManager_( new ActionManager( this ) ) 
                                 , pBrokerSessionEvent_( 0 )
                                 , brokerSession_( 0 ) 
                                 , dataprocFactory_( 0 )
{
    instance_ = this;
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

    iSequence_.reset( new iSequenceImpl );
    if ( iSequence_ && install_isequence( config, apppath, *iSequence_ ) ) {
        addObject( iSequence_.get() );
    }

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
                if ( mimeTypeHelper::add( mdb, xml.c_str(), xml.size(), error_message ) )
					mimeTypeHelper::populate( mTypes, xml.c_str() );
            });


        // core mime-types
        if ( ! mdb->addMimeTypes(":/dataproc/mimetype.xml", error_message) )
            adportable::debug( __FILE__, __LINE__ ) << "addMimeTypes" << ":/dataproc/mimetype.xml" << error_message;


        dataprocFactory_ = new DataprocessorFactory( this, mTypes );
        addAutoReleasedObject( dataprocFactory_ );
    } while ( 0 );

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

	std::wstring filename = token.toStdWString();

    SessionManager::vector_type::iterator it = SessionManager::instance()->find( filename );
    if ( it == SessionManager::instance()->end() ) {
		Core::EditorManager::instance()->openEditor( token );
        it = SessionManager::instance()->find( filename );
    }
    if ( it != SessionManager::instance()->end() ) {
		Dataprocessor& processor = it->getDataprocessor();
		processor.load( path.toStdWString(), id.toStdWString() );
    }
}

void
DataprocPlugin::onSelectTimeRangeOnChromatogram( double x1, double x2 )
{
	waitCursor w;

	Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor();
	if ( dp ) {
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
					while ( pos < pos2	&& dset->getSpectrum( 0, pos++, a ) ) {
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

				// add centroid spectrum if exist (Bruker's compassXtract returns centroid as 2nd function)
				if ( folium ) {
                    bool hasCentroid( false );
                    if ( pos1 == pos2 && dset->getFunctionCount() >= 2 ) {
                        adcontrols::MassSpectrumPtr pCentroid( new adcontrols::MassSpectrum );
                        if ( dset->getSpectrum( 1, pos1, *pCentroid ) ) {
                            hasCentroid = true;
                            portfolio::Folium att = folium.addAttachment( L"Centroid Spectrum" );
                            att.assign( pCentroid, pCentroid->dataClass() );
                            SessionManager::instance()->updateDataprocessor( dp, folium );
                        }
                    }
                    if ( ! hasCentroid ) {
                        mainWindow_->getProcessMethod( m );
                        dp->applyProcess( folium, m, CentroidProcess );
                    }
				}
			}
		}
	}
}

void
DataprocPlugin::handleCreateChromatograms( const adcontrols::MassSpectrum& ms, double lMass, double hMass )
{
	Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor();
    const adcontrols::LCMSDataset * ds( 0 );
	if ( dp && ( ds = dp->getLCMSDataset() ) ) {
        using namespace adcontrols;
        double width = 0.1;
        adcontrols::ProcessMethod pm;
        mainWindow_->getProcessMethod( pm );
        const TargetingMethod * p = pm.find< TargetingMethod >();
        if ( p )
            width = p->peak_width();

        adportable::array_wrapper< const double > masses( ms.getMassArray(), ms.size() );
        adportable::array_wrapper< const double > intens( ms.getIntensityArray(), ms.size() );
		auto lIt = std::upper_bound( masses.begin(), masses.end(), lMass );
		auto hIt = std::upper_bound( masses.begin(), masses.end(), hMass );
		std::vector< unsigned int > vec;
		for ( auto it = lIt; it != hIt; ++it )
			vec.push_back( std::distance( masses.begin(), it ) );
		std::vector< std::pair< double, double > > list;
		while ( list.size() < 3 && !vec.empty() ) {
			auto it = std::max_element( vec.begin(), vec.end()
                                        , [&]( unsigned int a, unsigned int b ){ return intens[b] > intens[a]; } );
			list.push_back( std::pair<double, double>( masses[ *it ], width ) );
			vec.erase( it );
		}

        QProgressBar progressBar;
        progressBar.setVisible( true );
		std::vector< adcontrols::Chromatogram> chromatograms;
		std::function <bool(long, long)> callback = [&](long curr, long total)->bool{
			if ( curr == 0 )
				progressBar.setRange( 0, total );
			progressBar.setValue( curr );
			return false;
		};
        adcontrols::ProcessMethod m;
        mainWindow_->getProcessMethod( m );
		if ( ds->getChromatograms( 0, list, chromatograms, callback ) ) {
            for ( const auto& c: chromatograms )
                dp->addChromatogram( c, m );
        }
    }
}

void
DataprocPlugin::extensionsInitialized()
{
#if 0
	Broker::Manager_var mgr = adorbmgr::orbmgr::getBrokerManager();
    if ( ! CORBA::is_nil( mgr ) ) {
        brokerSession_ = mgr->getSession( L"acquire" );
        pBrokerSessionEvent_ = new QBrokerSessionEvent;
        brokerSession_->connect( "-user-", "-password-", "dataproc", pBrokerSessionEvent_->_this() );
        connect( pBrokerSessionEvent_, SIGNAL( signal_portfolio_created( const QString ) )
                 , this, SLOT(handle_portfolio_created( const QString )) );
        connect( pBrokerSessionEvent_, SIGNAL( signal_folium_added( const QString, const QString, const QString ) )
                 , this, SLOT(handle_folium_added( const QString, const QString, const QString )) );
    } else {
        QMessageBox::critical( 0, "DataprocPlugin::extensionsInitialized"
                               , "can't find ior for adbroker -- maybe servant plugin load failed.");
    }
#endif
    mainWindow_->OnInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
DataprocPlugin::aboutToShutdown()
{
    adportable::debug(__FILE__, __LINE__) << "====== DataprocPlugin shutting down...  ===============";

    mainWindow_->OnFinalClose();
#if 0
	if ( ! CORBA::is_nil( brokerSession_ ) ) {

        disconnect( pBrokerSessionEvent_, SIGNAL( signal_portfolio_created( const QString ) )
                    , this, SLOT(handle_portfolio_created( const QString )) );

        disconnect( pBrokerSessionEvent_, SIGNAL( signal_folium_added( const QString, const QString, const QString ) )
                    , this, SLOT(handle_folium_added( const QString, const QString, const QString )) );

        brokerSession_->disconnect( pBrokerSessionEvent_->_this() );

        // destruct event sink object -->
        CORBA::release( pBrokerSessionEvent_->_this() ); // delete object reference
        adorbmgr::orbmgr::deactivate( pBrokerSessionEvent_ );

        delete pBrokerSessionEvent_;
        pBrokerSessionEvent_ = 0;
        // <-- end event sink desctruction

        CORBA::release( brokerSession_ );
        brokerSession_ = 0;
    }
#endif
    adportable::debug(__FILE__, __LINE__) << "====== DataprocPlugin shutdown complete ===============";
	return SynchronousShutdown;
}

bool
DataprocPlugin::install_isequence( const adportable::Configuration& config
                                   , const std::wstring& apppath
                                   , iSequenceImpl& impl )
{
    using adportable::Configuration;
    const Configuration * tab = Configuration::find( config, "ProcessMethodEditors" );    
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
