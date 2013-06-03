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

#include "sequenceplugin.hpp"
#include "constants.hpp"
#include "mode.hpp"
#include "mainwindow.hpp"
#include "sequenceeditorfactory.hpp"
//#include <adplugin/adplugin.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/constants.hpp>
#include <adportable/configuration.hpp>
#include <qtwrapper/qstring.hpp>
#include <qtwrapper/application.hpp>

#include <QtCore/qplugin.h>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/modemanager.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>
#include <utils/fancymainwindow.h>

#if QT_VERSION >= 0x050000
# include <QtWidgets/QHBoxLayout>
# include <QtWidgets/QBoxLayout>
# include <QtWidgets/QToolButton>
# include <QtWidgets/QLabel>
#else
# include <QtGui/QHBoxLayout>
# include <QtGui/QBoxLayout>
# include <QtGui/QToolButton>
# include <QtGui/QLabel>
#endif

#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QStringList>
#include <QDir>
#include <QtCore>
#include <vector>
#include <algorithm>

using namespace sequence;
using namespace sequence::internal;

SequencePlugin::SequencePlugin() : mainWindow_( new MainWindow )
{
}

SequencePlugin::~SequencePlugin()
{
	if ( mode_ )
        removeObject( mode_.get() );
}

bool
SequencePlugin::initialize(const QStringList& arguments, QString* error_message)
{
    Q_UNUSED( arguments );
    
    Core::ICore * core = Core::ICore::instance();
    if ( core == 0 )
        return false;
    
    QList<int> context;
    Core::UniqueIDManager * uidm = core->uniqueIDManager();
    if ( uidm ) {
        context.append( uidm->uniqueIdentifier( QLatin1String("Sequence.MainView") ) );
        context.append( uidm->uniqueIdentifier( Core::Constants::C_NAVIGATION_PANE ) );
    }

    //---------
    std::wstring pluginpath = qtwrapper::application::path( L".." );  // remove 'bin' from "~/qtplatz/bin"
    
    Core::MimeDatabase* mdb = core->mimeDatabase();
    if ( ! ( mdb && mdb->addMimeTypes(":/sequence/sequence-mimetype.xml", error_message) ) )
        return false;

    Core::ActionManager * am = core->actionManager();
    if ( am ) {
        Core::Command* cmd = am->registerAction( new QAction(this)
                                                 , "SequencePlugin.FILE_NEW"
                                                 , QList<int>() << Core::Constants::C_GLOBAL_ID );
        cmd->action()->setText("New Sample Sequence");
        am->actionContainer( Core::Constants::M_FILE)->addAction( cmd );
        connect( cmd->action(), SIGNAL( triggered(bool) ), this, SLOT( handleFileNew( bool ) ) );
    }

    // expose editor factory for sequence (table)
    addAutoReleasedObject( new SequenceEditorFactory(this) );

    mode_.reset( new Mode( this ) );
    if ( ! mode_ )
        return false;
    
    if ( ! mainWindow_ )
        return false;

    Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
    mainWindow_->activateLayout();
    mainWindow_->createActions();
    QWidget * widget = mainWindow_->createContents( mode_.get() );

    mode_->setWidget( widget );
    addObject( mode_.get() );

    return true;
}

void
SequencePlugin::extensionsInitialized()
{
	mainWindow_->OnInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
SequencePlugin::aboutToShutdown()
{
    mainWindow_->OnFinalClose();
	return SynchronousShutdown;
}

void
SequencePlugin::handleFileNew( bool )
{
	Core::EditorManager * em = Core::ICore::instance()->editorManager();
    if ( em ) {
        QString pattern;
		Core::IEditor * editor = em->openEditorWithContents( Constants::C_SEQUENCE, &pattern, "" );
		em->activateEditor( editor );
    }
}

Q_EXPORT_PLUGIN( SequencePlugin )
