// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include "quaneditor.hpp"
#include "quandocument.hpp"
#include "quanconstants.hpp"
#include <coreplugin/modemanager.h>
#include <QWidget>
#include <QEvent>

namespace quan {

    class QuanEditor::impl {
    public:
        impl() : file_( std::make_unique< QuanDocument >() )
               , widget_( 0 ) {
        }
        ~impl() {
            delete widget_;
        }
        QWidget * widget_;
        std::unique_ptr< QuanDocument > file_;
    };

}


using namespace quan;

QuanEditor::~QuanEditor()
{
}

QuanEditor::QuanEditor( QObject * parent ) : impl_( std::make_unique< impl >() )
{
    impl_->widget_ = new QWidget;
    // widget_->installEventFilter( this );
    setWidget( impl_->widget_ );
}


Core::IDocument *
QuanEditor::document() const
{
    return impl_->file_.get();
}


QWidget *
QuanEditor::toolBar()
{
    return 0;
}

Core::IEditor *
QuanEditor::duplicate()
{
    return 0;
}

#if 0
bool
QuanEditor::eventFilter( QObject * object, QEvent * event )
{
    if ( object == widget_ ) {
        if ( event->type() == QEvent::ShowToParent )
            Core::ModeManager::activateMode( Constants::C_QUAN );
    }
    return false;
}

bool
QuanEditor::open( QString*, const QString&, const QString& )
{
    return true;
}

Core::IDocument *
QuanEditor::document()
{
    return proxy_;
}

QByteArray
QuanEditor::saveState() const
{
    return QByteArray();
}

bool
QuanEditor::restoreState( const QByteArray & )
{
    return true;
}


QWidget *
QuanEditor::toolBar()
{
    return 0;
}

Core::Context
QuanEditor::context() const
{
    return Core::Context( Constants::C_QUAN );
}

bool
QuanDocProxy::isSaveAsAllowed() const
{
    return true;
}

bool
QuanDocProxy::save( QString * /*errorString*/, const QString& /*filename*/, bool /* autoSave */)
{
    return true;
}

QString
QuanDocProxy::defaultPath() const
{
    return QString();
}

bool
QuanDocProxy::reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType )
{
    return true;
}

QString
QuanDocProxy::suggestedFileName() const
{
    return QString();
}

bool
QuanDocProxy::isModified() const
{
    return false;
}

bool
QuanDocProxy::isFileReadOnly() const
{
    return false;
}
#endif
