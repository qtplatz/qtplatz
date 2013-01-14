/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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
    
    Core::MimeDatabase* mdb = core->mimeDatabase();
    if ( mdb ) {
        if ( !mdb->addMimeTypes(":/sequence/sequence-mimetype.xml", error_message) )
            return false;
    }
    // expose editor factory for sequence (table)
    addAutoReleasedObject( new SequenceEditorFactory(this) );

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
/*
	Core::EditorManager * em = Core::ICore::instance()->editorManager();
	SequenceEditorFactory * factory = ExtensionSystem::PluginManager::instance()->getObject< SequenceEditorFactory >();
    if ( factory ) {
		Core::IEditor * ieditor = factory->createEditor( 0 );
		em->pushEditor( ieditor );
    }
*/
}

void
SequencePlugin::shutdown()
{
    mainWindow_->OnFinalClose();
}

Q_EXPORT_PLUGIN( SequencePlugin )
