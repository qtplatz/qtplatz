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

#include "clustereditor.hpp"
#include "constants.hpp"
#include <coreplugin/modemanager.h>
#include <QWidget>
#include <QEvent>

using namespace cluster;

ClusterEditor::~ClusterEditor()
{
}

ClusterEditor::ClusterEditor( QObject * parent ) : Core::IEditor( parent )
                                           , widget_( new QWidget )
                                           , proxy_( new ClusterDocProxy )
{
    widget_->installEventFilter( this );
    setWidget( widget_ );
}

bool
ClusterEditor::eventFilter( QObject * object, QEvent * event )
{
    if ( object == widget_ ) {
        if ( event->type() == QEvent::ShowToParent )
            Core::ModeManager::activateMode( Constants::C_CLUSTER_MODE );
    }
    return false;
}

bool
ClusterEditor::open( QString*, const QString&, const QString& )
{
    return true;
}

Core::IDocument *
ClusterEditor::document()
{
    return proxy_;
}

QByteArray
ClusterEditor::saveState() const
{
    return QByteArray();
}

bool
ClusterEditor::restoreState( const QByteArray & )
{
    return true;
}


QWidget *
ClusterEditor::toolBar()
{
    return 0;
}

Core::Context
ClusterEditor::context() const
{
    return Core::Context( Constants::C_CLUSTER_MODE );
}

bool
ClusterDocProxy::isSaveAsAllowed() const
{
    return true;
}

bool
ClusterDocProxy::save( QString * /*errorString*/, const QString& /*filename*/, bool /* autoSave */)
{
    return true;
}

QString
ClusterDocProxy::defaultPath() const
{
    return QString();
}

bool
ClusterDocProxy::reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType )
{
    return true;
}

QString
ClusterDocProxy::suggestedFileName() const
{
    return QString();
}

bool
ClusterDocProxy::isModified() const
{
    return false;
}

bool
ClusterDocProxy::isFileReadOnly() const
{
    return false;
}
