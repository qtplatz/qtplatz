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
		MainWindow& mainWindow_;
		std::vector< adextension::iEditorFactory * > factories_;
	public:
		~SequenceAdapter() {
            for ( size_t i = 0; i < factories_.size(); ++i )
				delete factories_[ i ];
		}
		SequenceAdapter( MainWindow& mainWindow ) : mainWindow_( mainWindow ) {
		}
		virtual void addEditorFactory( adextension::iEditorFactory * p ) {
			factories_.push_back( p ); // keep pointer for delete on close
			mainWindow_.createDockWidget( *p );
		};
		virtual void removeEditorFactory( adextension::iEditorFactory * p ) {
			std::vector< adextension::iEditorFactory * >::iterator it = std::remove( factories_.begin(), factories_.end(), p );
			if ( it != factories_.end() )
				factories_.erase( it );
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
                                 , adapter_( new SequenceAdapter( *mainWindow_ ) )
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
    }
    // expose editor factory for sequence (table)
    addAutoReleasedObject( new SequenceEditorFactory(this) );

    // expose SequenceAdapter for method editor factories
    if ( adapter_ )
        addObject( adapter_.get() );

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
	// mainWindow_->createDockWidgets( apppath, acquire_config, dataproc_config );

    mode_->setWidget( widget );
    addObject( mode_.get() );

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
