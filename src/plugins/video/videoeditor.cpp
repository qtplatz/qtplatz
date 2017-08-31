// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
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

#include "videoeditor.hpp"
#include "constants.hpp"
#include "document.hpp"
#include <coreplugin/documentmanager.h>
#include <adportable/debug.hpp>
#include <coreplugin/modemanager.h>
#include <coreplugin/idocument.h>
#include <coreplugin/editormanager/ieditor.h>
#include <QWidget>
#include <QEvent>

namespace video {

    class VideoEditor::Document : public Core::IDocument {
        Q_OBJECT
    public:
        Document();
        ~Document();

        // Core::IDocument
        bool save( QString* errorString, const QString& filename = QString(), bool autoSave = false ) override;
        bool reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType ) override;

        QString defaultPath() const override;
        QString suggestedFileName() const override;
        bool isModified() const override;
        bool isSaveAsAllowed() const override;
        bool isFileReadOnly() const override;
    };
};

using namespace video;

VideoEditor::~VideoEditor()
{
}

VideoEditor::VideoEditor( QObject * parent ) : Core::IEditor( parent )
                                             , doc_( new Document )
{
    setWidget( new QWidget );
    widget()->installEventFilter( this );
}

bool
VideoEditor::eventFilter( QObject * object, QEvent * event )
{
    if ( object == widget() ) {
        if ( event->type() == QEvent::ShowToParent )
            Core::ModeManager::activateMode( Constants::C_VIDEO_MODE );
    }
    return false;
}

bool
VideoEditor::open( QString* errorMessage, const QString& filename, const QString& )
{
    if ( document::instance()->openFile( filename, *errorMessage ) ) {
        Core::DocumentManager::addToRecentFiles( filename );
        return true;
    }
    return false;
}

Core::IDocument *
VideoEditor::document()
{
    return doc_;
}

QByteArray
VideoEditor::saveState() const
{
    return QByteArray();
}

bool
VideoEditor::restoreState( const QByteArray & )
{
    return true;
}


QWidget *
VideoEditor::toolBar()
{
    return 0;
}

Core::Context
VideoEditor::context() const
{
    return Core::Context( Constants::C_VIDEO_MODE );
}


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

VideoEditor::Document::Document()
{
    setId( Constants::C_VIDEO_MODE );
}

VideoEditor::Document::~Document()
{
}

bool
VideoEditor::Document::save( QString* errorString, const QString& filename, bool autoSave )
{
}

bool
VideoEditor::Document::reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType )
{
}

QString
VideoEditor::Document::defaultPath() const
{
    return QString();
}

QString
VideoEditor::Document::suggestedFileName() const
{
    return QString();
}

bool
VideoEditor::Document::isModified() const
{
    return false;
}

bool
VideoEditor::Document::isSaveAsAllowed() const
{
    return true;
}

bool
VideoEditor::Document::isFileReadOnly() const
{
    return false;
}

#include "videoeditor.moc"

