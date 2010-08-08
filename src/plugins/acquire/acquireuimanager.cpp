//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "acquireuimanager.h"
#include "acquireactions.h"
#include <adplugin/ifactory.h>

#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>

#include <adwidgets/dataplot.h>
#include <QDockWidget>
#include <utils/fancymainwindow.h>
#include <utils/styledbar.h>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QTextEdit>
#include <QPluginLoader>
#include <QLibrary>
#include <QtCore>
#include <QUrl>

// #include <qtwrapper/xmldom.h> // using Qt native XML parser
#include <qtwrapper/qstring.h>
#include <xmlwrapper/xmldom.h>

namespace Acquire { 
  namespace internal {

    struct AcquireUIManagerData : boost::noncopyable {
      AcquireUIManagerData() : mainWindow_(0) {}
      
      Utils::FancyMainWindow* mainWindow_;

      std::vector< QDockWidget * > dockWidgetVec_;
    };

  }
}

using namespace Acquire;
using namespace Acquire::internal;

AcquireUIManager::~AcquireUIManager()
{
  delete d_;
}

AcquireUIManager::AcquireUIManager(QObject *parent) : QObject(parent)
						    , d_(0)
{
  d_ = new AcquireUIManagerData();
}

QMainWindow *
AcquireUIManager::mainWindow() const
{
  return d_->mainWindow_;
}

void
AcquireUIManager::init()
{
    if ( ! d_ )
        return;

    Acquire::internal::AcquireUIManagerData& m = *d_;

    QDir dir = QCoreApplication::instance()->applicationDirPath();
    dir.cdUp();
    dir.cd( "lib/qtPlatz/plugins/ScienceLiaison" );

    QString configFile = dir.path() + "/acquire.config.xml";

    using namespace xmlwrapper;
    using namespace xmlwrapper::msxml;

    XMLDocument config;

    do {
        if ( config.load( qtwrapper::wstring( configFile ) ) ) {
            XMLNodeList widgets = config.selectNodes( L"./AcquireConfiguration//Configuration[@type='adplugin']" );
            XMLNodeList tabs = config.selectNodes( L"./AcquireConfiguration//Configuration[@name='instrument_monitor_tab']/Tab/Item" );
            for ( unsigned int i = 0; i < tabs.size(); ++i ) {
                XMLNode& node = tabs[i];
                std::wstring name = node.attribute( L"name" );
                std::wstring component = node.attribute( L"component" );
                bool readOnly = node.attribute( L"readonly" ) == L"true" ? true : false;
                std::wstring text = node.textValue();
                std::wstring option = node.attribute( L"option" );
            }
        }
    } while(0);
  
#if defined _DEBUG
  QString adtofms = dir.path() + "/adtofmsd.dll";
#else
  QString adtofms = dir.path() + "/adtofms.dll";
#endif

  QPluginLoader loader( adtofms, this );
  if ( ! loader.load() ) {
	  loader.setFileName( adtofms );
	  if ( ! loader.load() ) {
		  QString error = loader.errorString();
	  }
  }
  QObject * instance = loader.instance();
  if ( instance ) {
	  adplugin::IFactory* piFactory = qobject_cast< adplugin::IFactory *>( instance );
	  QWidget * pWidget = piFactory->create_widget( "adplugin.ui.IMonitor" );
  }
  
  m.mainWindow_ = new Utils::FancyMainWindow;
  if ( d_ && m.mainWindow_ ) {
    m.mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    m.mainWindow_->setDocumentMode( true );
    
    QWidget * edit1 = new QTextEdit( "Sequence" );
    edit1->setWindowTitle( tr("Sequence") );

    QWidget * edit2 = new QTextEdit( "Log" );
    edit2->setWindowTitle( tr("Log Book") );

    QWidget * edit3 = new QTextEdit( "MS" );
    edit3->setWindowTitle( tr("Autosampler") );

    QWidget * edit4 = new QTextEdit( "Edit4" );
    edit4->setWindowTitle( tr("Mass Spectrometer") );

    QWidget * edit5 = new QTextEdit( "Edit 5" );
    edit5->setWindowTitle( tr("Agilnet 1290") );

    QWidget * edit6 = new QTextEdit( "Edit 6" );
    edit6->setWindowTitle( tr("All") );

    QDockWidget * dock1 = m.mainWindow_->addDockForWidget( edit1 );
    QDockWidget * dock2 = m.mainWindow_->addDockForWidget( edit2 );
    QDockWidget * dock3 = m.mainWindow_->addDockForWidget( edit3 );
    QDockWidget * dock4 = m.mainWindow_->addDockForWidget( edit4 );
    QDockWidget * dock5 = m.mainWindow_->addDockForWidget( edit5 );
    QDockWidget * dock6 = m.mainWindow_->addDockForWidget( edit6 );
    
    m.dockWidgetVec_.push_back( dock1 );
    m.dockWidgetVec_.push_back( dock2 );
    m.dockWidgetVec_.push_back( dock3 );
    m.dockWidgetVec_.push_back( dock4 );
    m.dockWidgetVec_.push_back( dock5 );
    m.dockWidgetVec_.push_back( dock6 );
    
    // todo
    // set actions
  }
}

class setTrackingEnabled {
  Utils::FancyMainWindow& w_;
public:
  setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) {
    w_.setTrackingEnabled( false );
  }
  ~setTrackingEnabled() {
    w_.setTrackingEnabled( true );
  }
};

void
AcquireUIManager::setSimpleDockWidgetArrangement()
{
  AcquireUIManagerData& m = *d_;
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

  for ( unsigned int i = 2; i < m.dockWidgetVec_.size(); ++i )
    m.mainWindow_->tabifyDockWidget( m.dockWidgetVec_[1], m.dockWidgetVec_[i] );

}
