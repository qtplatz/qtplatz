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

#include "sequenceplugin.hpp"
#include "mode.hpp"
#include "mainwindow.hpp"
#include "sequenceeditorfactory.hpp"
#include <adextension/isequence.hpp>
#include <adextension/ieditorfactory.hpp>
#include <adplugin/adplugin.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/constants.hpp>
#include <adportable/configuration.hpp>
#include <qtwrapper/qstring.hpp>

#include <QtCore/qplugin.h>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/mimedatabase.h>

#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/modemanager.h>

#include <utils/styledbar.h>
#include <utils/fancymainwindow.h>

#include <QtGui/QHBoxLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QStringList>
#include <QDir>
#include <QtCore>
#include <vector>
#include <algorithm>

namespace sequence { namespace internal {

	class SequenceAdapter : public adextension::iSequence {
        SequencePlugin& plugin_;
		std::vector< adextension::iEditorFactory * > factories_;
	public:
		~SequenceAdapter() {
            for ( size_t i = 0; i < factories_.size(); ++i )
				delete factories_[ i ];
		}
		SequenceAdapter( SequencePlugin& plugin ) : plugin_( plugin ) {
		}
		virtual void addEditorFactory( adextension::iEditorFactory * p ) {
			factories_.push_back( p );
		};
		virtual void removeEditorFactory( adextension::iEditorFactory * p ) {
			factories_.erase( std::remove( factories_.begin(), factories_.end(), p ) );
		};
		//
		typedef std::vector< adextension::iEditorFactory * > vector_type;
		size_t size() const { return factories_.size(); }
		vector_type::iterator begin() { return factories_.begin(); }
		vector_type::iterator end() { return factories_.end(); }
	};

  }
}

using namespace sequence;
using namespace sequence::internal;

SequencePlugin::SequencePlugin() : mainWindow_( new MainWindow )
                                 , adapter_( new SequenceAdapter( *this ) )
{
}

SequencePlugin::~SequencePlugin()
{
    if ( mode_ )
        removeObject( mode_.get() );

	if ( adapter_ )
		removeObject( adapter_.get() );
}

/*
QWidget *
SequencePlugin::CreateSequenceWidget( const std::wstring& apppath, const adportable::Configuration& config )
{
    const adportable::Configuration * pSeq = adportable::Configuration::find( config, L"sequence_monitor" );
    if ( pSeq ) {
        const std::wstring name = pSeq->name();
        if ( pSeq->isPlugin() ) {
            QWidget * pWidget = adplugin::manager::widget_factory( *pSeq, apppath.c_str(), 0 );
            adplugin::LifeCycle * pLifeCycle = dynamic_cast< adplugin::LifeCycle *>( pWidget );
            if ( pLifeCycle )
                pLifeCycle->OnInitialUpdate();
            return pWidget;
        }
    }
    return 0;
}
*/

bool
SequencePlugin::initialize(const QStringList& arguments, QString* error_message)
{
    Q_UNUSED( arguments );
    
    Core::ICore * core = Core::ICore::instance();
    
    QList<int> context;
    if ( core ) {
        Core::UniqueIDManager * uidm = core->uniqueIDManager();
        if ( uidm ) {
            context.append( uidm->uniqueIdentifier( QLatin1String("Sequence.MainView") ) );
            context.append( uidm->uniqueIdentifier( Core::Constants::C_NAVIGATION_PANE ) );
        }
    } else
        return false;

    //---------
    QDir dir = QCoreApplication::instance()->applicationDirPath();
    dir.cdUp();
    std::wstring apppath = qtwrapper::wstring::copy( dir.path() );
    dir.cd( adpluginDirectory );
    std::wstring pluginpath = qtwrapper::wstring::copy( dir.path() );

    adportable::Configuration acquire_config;
    adportable::Configuration dataproc_config;
    do {
        std::wstring file = pluginpath + L"/acquire.config.xml";
        const wchar_t * query = L"/AcquireConfiguration/Configuration";
        adplugin::manager::instance()->loadConfig( acquire_config, file, query );
    } while(0);

    do {
        std::wstring file = pluginpath + L"/dataproc.config.xml";
        const wchar_t * query = L"/DataprocConfiguration/Configuration";
        adplugin::manager::instance()->loadConfig( dataproc_config, file, query );
    } while(0);
    //----
    
    Core::MimeDatabase* mdb = core->mimeDatabase();
    if ( mdb ) {
        if ( !mdb->addMimeTypes(":/sequence/sequence-mimetype.xml", error_message) )
            return false;
        addAutoReleasedObject( new SequenceEditorFactory(this) );
    }

    mode_.reset( new Mode( this ) );
    if ( ! mode_ )
        return false;
    
    // mainWindow_.reset( new MainWindow );
    if ( ! mainWindow_ )
        return false;

    Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
    mainWindow_->activateLayout();
    mainWindow_->createActions();
    QWidget * widget = mainWindow_->createContents( mode_.get() );
    mainWindow_->createDockWidgets( apppath, acquire_config, dataproc_config );

    mode_->setWidget( widget );
    addObject( mode_.get() );

    return true;

#if 0    
    mode_->setContext( context );
    manager_.reset( new SequenceManager(0) );
    if ( manager_ )
        manager_->init( apppath, acquire_config, dataproc_config );

    //              [mainWindow]
    // splitter> ---------------------
    //              [OutputPane]
    
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        splitter->addWidget( manager_->mainWindow() );
        splitter->addWidget( new Core::OutputPanePlaceHolder( mode ) );
        
        splitter->setStretchFactor( 0, 10 );
        splitter->setStretchFactor( 1, 0 );
        splitter->setOrientation( Qt::Vertical ); // horizontal splitter bar
    }

    //////////////////////////////////////////////////////////////
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
        //////////////////
    }
    Utils::StyledBar * toolBar2 = new Utils::StyledBar;
    if ( toolBar2 ) {
        toolBar2->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar2 );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        Core::ActionManager *am = core->actionManager();
        if ( am ) {
            toolBarLayout->addWidget( new QLabel( tr("AA") ) );
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget( new QLabel( tr("BB") ) );
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget( new QLabel( tr("CC") ) );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addWidget( new QLabel( tr("Threads:") ) );
    }
    
    /******************************************************************************
     */
    
    QWidget* centralWidget = new QWidget;
    manager_->mainWindow()->setCentralWidget( centralWidget );
    
    Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
    if ( splitter3 ) {
        QWidget * pSequence = CreateSequenceWidget( apppath, acquire_config );
        if ( pSequence )
            splitter3->addWidget( pSequence );
        else
            splitter3->addWidget( new QTextEdit );
    }
    
    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( toolBar );
    toolBarAddingLayout->addWidget( splitter3 );
    toolBarAddingLayout->addWidget( toolBar2 );
    
    mode->setWidget( splitter2 );
    
    //////////////////////////////////
    manager_->setSimpleDockWidgetArrangement();
    addAutoReleasedObject(mode);
    //////////////////////////////////
#endif
    return true;
}

void
SequencePlugin::extensionsInitialized()
{
    mainWindow_->OnInitialUpdate();
}

void
SequencePlugin::shutdown()
{
    mainWindow_->OnFinalClose();
}

Q_EXPORT_PLUGIN( SequencePlugin )
