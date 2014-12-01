/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "sequenceeditor.hpp"
#include "constants.hpp"
#include "sequencewnd.hpp"
#include "mainwindow.hpp"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/elementalcompositionmethod.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adsequence/sequence.hpp>
#include <adsequence/schema.hpp>
//#include <coreplugin/uniqueidmanager.h>
//#include <coreplugin/filemanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <qtwrapper/qstring.hpp>
#include <QEvent>

using namespace sequence;

SequenceEditor::~SequenceEditor()
{
    //delete file_;
    //delete widget_;
}

SequenceEditor::SequenceEditor(QObject *parent) : Core::IEditor(parent)
                                                , widget_( new QWidget )
                                                , proxy_( new SequenceDocProxy )

{
    widget_->installEventFilter( this );
    setWidget( widget_ );
#if 0
    widget_->OnInitialUpdate( file_->adsequence().schema() );
    bool res;	
	res = connect( widget_, SIGNAL( lineAdded( size_t ) ), this, SLOT( onLineAdded( size_t ) ) );
	assert( res );
	res = connect( widget_, SIGNAL( lineDeleted( size_t ) ), this, SLOT( onLineDeleted( size_t ) ) );
	assert( res );
	res = connect( widget_, SIGNAL( currentChanged( size_t, size_t ) ), this, SLOT( onCurrentChanged( size_t, size_t ) ) );
	assert( res );
#endif
}

bool
SequenceEditor::eventFilter( QObject * object, QEvent * event )
{
    if ( object == widget_ ) {
        if ( event->type() == QEvent::ShowToParent )
            Core::ModeManager::activateMode( Constants::C_SEQUENCE );
    }
    return false;
}

bool
SequenceEditor::open( QString*, const QString&, const QString& )
{
    return true;
}

Core::IDocument *
SequenceEditor::document()
{
    return proxy_;
}

QByteArray
SequenceEditor::saveState() const
{
    return QByteArray();
}

bool
SequenceEditor::restoreState( const QByteArray & )
{
    return true;
}


QWidget *
SequenceEditor::toolBar()
{
    return 0;
}

Core::Context
SequenceEditor::context() const
{
    return Core::Context( Constants::C_SEQUENCE );
}

///////////////////
bool
SequenceDocProxy::isSaveAsAllowed() const
{
    return true;
}

bool
SequenceDocProxy::save( QString * /*errorString*/, const QString& /*filename*/, bool /* autoSave */)
{
    return true;
}

QString
SequenceDocProxy::defaultPath() const
{
    return QString();
}

bool
SequenceDocProxy::reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType )
{
    return true;
}

QString
SequenceDocProxy::suggestedFileName() const
{
    return QString();
}

bool
SequenceDocProxy::isModified() const
{
    return false;
}

bool
SequenceDocProxy::isFileReadOnly() const
{
    return false;
}
