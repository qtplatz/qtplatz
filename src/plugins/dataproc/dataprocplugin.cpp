// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include "dataprocmode.hpp"
#include "dataprocmanager.hpp"
#include "dataprocessor.hpp"
#include "dataprocessorfactory.hpp"
#include "navigationwidgetfactory.hpp"
#include "sessionmanager.hpp"

#include "msprocessingwnd.hpp"
#include "elementalcompwnd.hpp"
#include "mscalibrationwnd.hpp"
#include "chromatogramwnd.hpp"

#include <QtCore/qplugin.h>
#include <QtCore>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/mimedatabase.h>
#include <QStringList>

#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/ieditor.h>

#include <utils/styledbar.h>
#include <utils/fancymainwindow.h>

#include <QtGui/QHBoxLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QDir>
#include <QMessageBox>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <qtwrapper/qstring.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adplugin/adplugin.hpp>
#include <adplugin/constants.hpp>

#include <acewrapper/constants.hpp>
#include <acewrapper/brokerhelper.hpp>
#include <acewrapper/input_buffer.hpp>
#include <adplugin/orbmanager.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/qbrokersessionevent.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <streambuf>
#include <fstream>
#include <iomanip>

#if defined _MSC_VER
# pragma warning(disable:4996)
#endif
# include <adinterface/brokerC.h>

using namespace dataproc::internal;

DataprocPlugin * DataprocPlugin::instance_ = 0;

DataprocPlugin::~DataprocPlugin()
{
}

DataprocPlugin::DataprocPlugin() : pSessionManager_( new SessionManager() )
                                 , pActionManager_( new ActionManager( this ) ) 
                                 , pBrokerSessionEvent_( 0 )
                                 , brokerSession_( 0 ) 
                                 , actionApply_( 0 )
                                 , currentFeature_( CentroidProcess )
{
    instance_ = this;
}

DataprocPlugin *
DataprocPlugin::instance()
{
    return instance_;
}

// static
static QToolButton * 
toolButton( QAction * action )
{
  QToolButton * button = new QToolButton;
  if ( button )
    button->setDefaultAction( action );
  return button;
}

bool
DataprocPlugin::initialize(const QStringList& arguments, QString* error_message)
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
    std::wstring apppath;
    do {
        QDir dir = QCoreApplication::instance()->applicationDirPath();
        dir.cdUp();
        apppath = qtwrapper::wstring::copy( dir.path() );
    } while(0);

    // std::wstring configFile = pluginpath + L"/dataproc.config.xml";
    std::wstring configFile = adplugin::orbLoader::config_fullpath( apppath, L"/ScienceLiaison/dataproc.config.xml" );
    const wchar_t * query = L"/DataprocConfiguration/Configuration";

    pConfig_.reset( new adportable::Configuration() );
    adportable::Configuration& config = *pConfig_;

    if ( ! adplugin::manager::instance()->loadConfig( config, configFile, query ) ) {
        error_message = new QString( "loadConfig load failed" );
        adportable::debug( __FILE__, __LINE__ ) << "loadConfig" << configFile << "failed";
    }
    //------------------------------------------------

    //------------------------------------------------

    DataprocessorFactory * dataprocFactory = 0;
    Core::MimeDatabase* mdb = core->mimeDatabase();
    if ( mdb ) {
        if ( !mdb->addMimeTypes(":/dataproc/dataproc-mimetype.xml", error_message) )
            return false;
        dataprocFactory = new DataprocessorFactory( this );
        addAutoReleasedObject( dataprocFactory );
    }

    DataprocMode * mode = new DataprocMode(this);
    if ( mode )
        mode->setContext( context );
    else
        return false;

    manager_.reset( new DataprocManager(0) );
    if ( manager_ )
        manager_->init( config, apppath );

    pActionManager_->initialize_actions( context );
    // initialize_actions();

    do {
    
        //              [mainWindow]
        // splitter> ---------------------
        //              [OutputPane] := ServantLog etc.
  
        Core::MiniSplitter * splitter = new Core::MiniSplitter;
        if ( splitter ) {
            splitter->addWidget( manager_->mainWindow() );
            splitter->addWidget( new Core::OutputPanePlaceHolder( mode ) );
            //splitter->addWidget( new QTextEdit("mainWindow") );
            //splitter->addWidget( new QTextEdit("This is edit" ) );
      
            splitter->setStretchFactor( 0, 10 );
            splitter->setStretchFactor( 1, 0 );
            splitter->setOrientation( Qt::Vertical ); // horizontal splitter bar
        }

        //
        //         <splitter2>         [mainWindow]
        // [Navigation] | [splitter ------------------- ]
        //                             [OutputPane]

        Core::MiniSplitter * splitter2 = new Core::MiniSplitter;
        if ( splitter2 ) {
            splitter2->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) );
            splitter2->addWidget( splitter );
            splitter2->setStretchFactor( 0, 0 );
            splitter2->setStretchFactor( 1, 1 );
        }
      
        Utils::StyledBar * toolBar = new Utils::StyledBar;
        if ( toolBar ) {
            toolBar->setProperty( "topBorder", true );
            QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
            toolBarLayout->setMargin(0);
            toolBarLayout->setSpacing(0);
            Core::ActionManager *am = core->actionManager();
            if ( am ) {
                /*
                toolBarLayout->addWidget(toolButton(am->command(Constants::CONNECT)->action()));
                toolBarLayout->addWidget(toolButton(am->command(Constants::INITIALRUN)->action()));
                toolBarLayout->addWidget(toolButton(am->command(Constants::RUN)->action()));
                toolBarLayout->addWidget(toolButton(am->command(Constants::STOP)->action()));
                toolBarLayout->addWidget(toolButton(am->command(Constants::ACQUISITION)->action()));
                */
            }
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget( new QLabel( tr("Sequence:") ) );
        }
        Utils::StyledBar * toolBar2 = new Utils::StyledBar;
        if ( toolBar2 ) {
            toolBar2->setProperty( "topBorder", true );
            QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar2 );
            toolBarLayout->setMargin(0);
            toolBarLayout->setSpacing(0);
            Core::ActionManager *am = core->actionManager();
            if ( am ) {
                QList<int> globalcontext;
                globalcontext << Core::Constants::C_GLOBAL_ID;

                actionApply_ = new QAction( QIcon( ":/dataproc/image/apply_small.png" ), tr("Apply" ), this );
                connect( actionApply_, SIGNAL( triggered() ), this, SLOT( actionApply() ) );
                am->registerAction( actionApply_, "dataproc.connect", globalcontext );
                toolBarLayout->addWidget( toolButton( am->command( "dataproc.connect" )->action() ) );
                /**/
                toolBarLayout->addWidget( new Utils::StyledSeparator );
                /**/
                QComboBox * features = new QComboBox;
                features->addItem( "Centroid" );
                features->addItem( "Isotope" );
                features->addItem( "Calibration" );
                toolBarLayout->addWidget( features );
                connect( features, SIGNAL( currentIndexChanged(int) ), this, SLOT( handleFeatureSelected(int) ) );
                connect( features, SIGNAL( activated(int) ), this, SLOT( handleFeatureActivated(int) ) );

                toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
            }
        }

        /******************************************************************************
        */

        QWidget* centralWidget = new QWidget;
        manager_->mainWindow()->setCentralWidget( centralWidget );

        std::vector< QWidget * > wnd;
        Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
        if ( splitter3 ) {
            QTabWidget * pTab = new QTabWidget;

            splitter3->addWidget( pTab );
            wnd.push_back( new MSProcessingWnd );
            pTab->addTab( wnd.back(), QIcon(":/acquire/images/debugger_stepoverproc_small.png"), "MS Processing" );
            wnd.push_back( new ElementalCompWnd );
            pTab->addTab( wnd.back(), QIcon(":/acquire/images/debugger_snapshot_small.png"), "Elemental Composition" );
            wnd.push_back( new MSCalibrationWnd( config, apppath ) );
            pTab->addTab( wnd.back(), QIcon(":/acquire/images/debugger_continue_small.png"), "MS Calibration" );
            wnd.push_back( new ChromatogramWnd( apppath ) );
            pTab->addTab( wnd.back(),  QIcon(":/acquire/images/watchpoint.png"), "Chromatogram" );

            if ( dataprocFactory )
                dataprocFactory->setEditor( pTab );
        }
        QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
        toolBarAddingLayout->setMargin(0);
        toolBarAddingLayout->setSpacing(0);
        toolBarAddingLayout->addWidget( toolBar );
        toolBarAddingLayout->addWidget( splitter3 );
        toolBarAddingLayout->addWidget( toolBar2 );

        mode->setWidget( splitter2 );

        // connections
        for ( std::vector< QWidget *>::iterator it = wnd.begin(); it != wnd.end(); ++it ) {
            connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) ), *it, SLOT( handleSessionAdded( Dataprocessor* ) ) );
            connect( SessionManager::instance(), SIGNAL( signalSelectionChanged( Dataprocessor*, portfolio::Folium& ) )
                     , *it, SLOT( handleSelectionChanged( Dataprocessor*, portfolio::Folium& ) ) );
        }
        connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) ), manager_.get(), SLOT( handleSessionAdded( Dataprocessor* ) ) );
        connect( SessionManager::instance(), SIGNAL( signalSelectionChanged( Dataprocessor*, portfolio::Folium& ) )
                 , manager_.get(), SLOT( handleSelectionChanged( Dataprocessor*, portfolio::Folium& ) ) );

    } while(0);
  
    manager_->setSimpleDockWidgetArrangement();
    addAutoReleasedObject(mode);

    addAutoReleasedObject( new NavigationWidgetFactory );

    return true;
}

void
DataprocPlugin::actionApply()
{
    adcontrols::ProcessMethod m;
    manager_->getProcessMethod( m );
    size_t n = m.size();
    if ( n > 0 ) {
        Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor();
        if ( processor ) {
            if ( currentFeature_ == internal::CalibrationProcess )
                processor->applyCalibration( m );
            else
                processor->applyProcess( m, currentFeature_ );
        }
    }
}

void
DataprocPlugin::handleFeatureSelected( int value )
{
    currentFeature_ = static_cast< ProcessType >( value );
}

void
DataprocPlugin::handleFeatureActivated( int value )
{
    currentFeature_ = static_cast< ProcessType >( value );
}

void
DataprocPlugin::handle_portfolio_created( const QString token )
{
    // simulate file->open()
#if defined DEBUG
    qDebug() << "DataprocPlugin::handle_portfolio_created(" << token << ")";
#endif
    boost::shared_ptr<Dataprocessor> processor( new Dataprocessor );
    if ( processor->create( token ) ) {
#if defined DEBUG
        qDebug() << "DataprocPlugin::handle_portfolio_created addDataprocessor(" << token << ")";
#endif
        SessionManager::instance()->addDataprocessor( processor );
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
#if 0
        //---------> for quick debug
        std::string name1( "C:/InfiTOF/" );
        std::string name2;
        do {
            time_t utc;
            time(&utc);
            struct tm tm = *localtime( &utc );
            name1 += ( boost::format( "%04d-%02d-%02d-%02d%02d%02d" ) % (tm.tm_year + 1900) % (tm.tm_mon + 1) % tm.tm_mday % tm.tm_hour % tm.tm_min % tm.tm_sec ).str();
            name2 = name1;
            name1 += ".txt";
            name2 += ".qtms";
        } while(0);

        std::ofstream of( name1.c_str() );
        if ( ! of.fail() ) {
            size_t size = ms.size();
            const double * pIntens = ms.getIntensityArray();
            const double * pMasses = ms.getMassArray();
            const adcontrols::MSProperty& prop = ms.getMSProperty();
            unsigned long sampInterval = prop.instSamplingInterval();
            unsigned long nDelay = prop.instSamplingStartDelay();
            for ( size_t i = 0; i < size; i++ ) {
                of << std::setprecision(12) << (double( ( nDelay + i ) * sampInterval ) * 1.0e-12 ) << "\t" << pMasses[i] << "\t" << pIntens[i] << std::endl;
            }
        }
        std::ofstream of2( name2.c_str(), std::ios_base::binary | std::ios_base::out );
        if ( ! of2.fail() ) {
            of2.write( reinterpret_cast< const char *>( var->serialized.get_buffer() ), var->serialized.length() );
        }
        //<--------------
#endif
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
            QMessageBox::critical( 0, "DataprocPlugin::extensionsInitialized", "can't find ior for adbroker -- maybe servant plugin load failed.");
        }
    } while(0);

    manager_->OnInitialUpdate();
}

void
DataprocPlugin::shutdown()
{
    adportable::debug(__FILE__, __LINE__) << "====== DataprocPlugin shutting down...  ===============";

    manager_->OnFinalClose();

    if ( ! CORBA::is_nil( brokerSession_ ) ) {

        disconnect( pBrokerSessionEvent_, SIGNAL( signal_portfolio_created( const QString ) ), this, SLOT(handle_portfolio_created( const QString )) );
        disconnect( pBrokerSessionEvent_, SIGNAL( signal_folium_added( const QString, const QString, const QString ) ), this, SLOT(handle_folium_added( const QString, const QString, const QString )) );

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
}


Q_EXPORT_PLUGIN( DataprocPlugin )
