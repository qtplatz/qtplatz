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

#include "dataprocmanager.hpp"
#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include <adportable/configuration.hpp>
#include <adcontrols/datafilebroker.hpp>
#include <adcontrols/datafile.hpp>
#include <adplugin/adplugin.hpp>
#include <adplugin/lifecycle.hpp>
#include <portfolio/folium.hpp>
#include <qtwrapper/qstring.hpp>
#include <boost/noncopyable.hpp>

#include <utils/fancymainwindow.h>
#include <utils/styledbar.h>
#include <QtCore/QHash>
#include <QString>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QTextEdit>
#include <QTabBar>
#include <QMessageBox>
#include <vector>
#if defined DEBUG
# include <iostream>
#endif

using namespace dataproc::internal;

namespace dataproc {
    namespace internal {
        class DataprocManagerImpl : boost::noncopyable {
        public:
            ~DataprocManagerImpl();
            DataprocManagerImpl();

            Utils::FancyMainWindow * mainWindow_;
            void init();

        public:
            //std::vector< QDockWidget * > dockWidgetVec_;
    };
  }
}

DataprocManager::~DataprocManager()
{
    delete pImpl_;
}

DataprocManager::DataprocManager(QObject *parent) : QObject(parent)
                                                  , pImpl_( new DataprocManagerImpl() )
{
}

QMainWindow *
DataprocManager::mainWindow() const
{
    return pImpl_->mainWindow_;
}

void
DataprocManager::init( const adportable::Configuration& config, const std::wstring& apppath )
{
    pImpl_->init();

    DataprocManagerImpl& m = *pImpl_;

    const adportable::Configuration * pTab = adportable::Configuration::find( config, L"ProcessMethodEditors" );
    if ( pTab ) {
        using namespace adportable;
        using namespace adplugin;
            
        // std::wstring loadpath = qtwrapper::wstring( dir.path() );
        // tab pages
#ifdef DEBUG
	std::cout << "------------- creating process method tab" << std::endl;
#endif
        for ( Configuration::vector_type::const_iterator it = pTab->begin(); it != pTab->end(); ++it ) {

            const std::wstring name = it->name();
#if defined DEBUG
	    std::wcout << "#####" << name << "#####" << std::endl;
#endif
            if ( it->isPlugin() ) {
                QWidget * pWidget = manager::widget_factory( *it, apppath.c_str(), 0 );
                if ( pWidget ) {
                    // query process method
                    connect( this, SIGNAL( signalGetProcessMethod( adcontrols::ProcessMethod& ) )
                             , pWidget, SLOT( getContents( adcontrols::ProcessMethod& ) ), Qt::DirectConnection );

                    pWidget->setMinimumHeight( 80 );

                    pWidget->setWindowTitle( qtwrapper::qstring( it->title() ) );
                    m.mainWindow_->addDockForWidget( pWidget );
                } else {
		    QMessageBox::critical(0, QLatin1String("dataprocmanager"), qtwrapper::qstring::copy(it->name()) );
		}
            }
        }
	std::cout << "------------- end process method tab" << std::endl;
    }       

    ////
    const adportable::Configuration * provider = adportable::Configuration::find( config, L"dataproviders" );
    if ( provider ) {
        using namespace adportable;
        using namespace adplugin;

        for ( Configuration::vector_type::const_iterator it = provider->begin(); it != provider->end(); ++it ) {
            const std::wstring name = apppath + it->module().library_filename();
            adcontrols::datafileBroker::register_library( name );
        }
    }
}

void
DataprocManager::OnInitialUpdate()
{
    DataprocManagerImpl& m = *pImpl_;

    QList< QDockWidget *> dockWidgets = m.mainWindow_->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        QObjectList list = dockWidget->children();
        foreach ( QObject * obj, list ) {
            adplugin::LifeCycle * pLifeCycle = dynamic_cast<adplugin::LifeCycle *>( obj );
            if ( pLifeCycle ) {
                pLifeCycle->OnInitialUpdate();
            }
        }
    }
}

void
DataprocManager::OnFinalClose()
{
    DataprocManagerImpl& m = *pImpl_;

    QList< QDockWidget *> dockWidgets = m.mainWindow_->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        QObjectList list = dockWidget->children();
        foreach ( QObject * obj, list ) {
            adplugin::LifeCycle * pLifeCycle = dynamic_cast<adplugin::LifeCycle *>( obj );
            if ( pLifeCycle ) {
                pLifeCycle->OnFinalClose();
            }
        }
    }
}

///////////////////////
DataprocManagerImpl::~DataprocManagerImpl()
{
}

DataprocManagerImpl::DataprocManagerImpl() : mainWindow_(0)
{
}

void
DataprocManagerImpl::init()
{
    mainWindow_ = new Utils::FancyMainWindow;
    if ( mainWindow_ ) {
        mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
        mainWindow_->setDocumentMode( true );
    }
}

void
DataprocManager::setSimpleDockWidgetArrangement()
{
    class setTrackingEnabled {
        Utils::FancyMainWindow& w_;
    public:
        setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
        ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
    };

    DataprocManagerImpl& m = *pImpl_;
    setTrackingEnabled lock( *m.mainWindow_ );
  
    QList< QDockWidget *> dockWidgets = m.mainWindow_->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        dockWidget->setFloating( false );
        m.mainWindow_->removeDockWidget( dockWidget );
    }

    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        m.mainWindow_->addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        dockWidget->show();
    }

    for ( int i = 1; i < dockWidgets.size(); ++i )
        m.mainWindow_->tabifyDockWidget( dockWidgets[0], dockWidgets[i] );

    QList< QTabBar * > tabBars = m.mainWindow_->findChildren< QTabBar * >();
    foreach( QTabBar * tabBar, tabBars ) 
        tabBar->setCurrentIndex( 0 );
}

void
DataprocManager::handleSessionAdded( dataproc::Dataprocessor * processor )
{
    adcontrols::datafile& file = processor->file();
    emit signalUpdateFile( &file );
}

void
DataprocManager::handleSelectionChanged( dataproc::Dataprocessor *, portfolio::Folium& )
{
}

void
DataprocManager::handleApplyMethod()
{
    // connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) ), pWidget, SLOT( handleSessionAdded( Dataprocessor* ) ) );
}

void
DataprocManager::getProcessMethod( adcontrols::ProcessMethod& pm )
{
    emit signalGetProcessMethod( pm );
}
