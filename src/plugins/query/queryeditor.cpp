// This is a -*- C++ -*- header.
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
1** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "queryeditor.hpp"
#include "queryconstants.hpp"
#include <coreplugin/modemanager.h>
#include <QWidget>
#include <QEvent>

using namespace query;

QueryEditor::~QueryEditor()
{
}

QueryEditor::QueryEditor( QObject * parent ) : Core::IEditor( parent )
                                           , widget_( new QWidget ) 
                                           , proxy_( new QueryDocProxy )
{
    widget_->installEventFilter( this );
    setWidget( widget_ );
}

bool
QueryEditor::eventFilter( QObject * object, QEvent * event )
{
    if ( object == widget_ ) {
        if ( event->type() == QEvent::ShowToParent )
            Core::ModeManager::activateMode( Constants::C_QUERY );
    }
    return false;
}

bool
QueryEditor::open( QString*, const QString&, const QString& )
{
    return true;
}

Core::IDocument *
QueryEditor::document()
{
    return proxy_;
}

QByteArray
QueryEditor::saveState() const
{
    return QByteArray();
}

bool
QueryEditor::restoreState( const QByteArray & )
{
    return true;
}


QWidget *
QueryEditor::toolBar()
{
    return 0;
}

Core::Context
QueryEditor::context() const
{
    return Core::Context( Constants::C_QUERY );
}

bool
QueryDocProxy::isSaveAsAllowed() const
{
    return true;
}

bool
QueryDocProxy::save( QString * /*errorString*/, const QString& /*filename*/, bool /* autoSave */)
{
    return true;
}

QString
QueryDocProxy::defaultPath() const
{
    return QString();
}

bool
QueryDocProxy::reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType )
{
    return true;
}

QString
QueryDocProxy::suggestedFileName() const
{
    return QString();
}

bool
QueryDocProxy::isModified() const
{
    return false;
}

bool
QueryDocProxy::isFileReadOnly() const
{
    return false;
}

