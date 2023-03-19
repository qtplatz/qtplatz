// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "dataproceditor.hpp"
#include "document.hpp"
#include "dataprocessor.hpp"
#include "dataprocfactory.hpp"
#include "dataprocconstants.hpp"
#include "mainwindow.hpp"
#include "msprocessingwnd.hpp"
#include "sessionmanager.hpp"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
# include <coreplugin/id.h>
#else
# include <utils/id.h>
#endif
#include <coreplugin/modemanager.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <qtwrapper/waitcursor.hpp>
#include <QTextEdit>
#include <QEvent>
#include <adportable/debug.hpp>

namespace dataproc {

    class DataprocEditor::impl {
    public:
        std::shared_ptr< Dataprocessor > file_;
        QWidget * widget_;
        impl() : widget_( new QWidget )
               , file_( Dataprocessor::make_dataprocessor() ) {
        }
        ~impl() {
            // delete widget_;
        }
    };
}

using namespace dataproc;

DataprocEditor::~DataprocEditor()
{
    SessionManager::instance()->removeEditor( this );
}

DataprocEditor::DataprocEditor() : impl_( std::make_unique< impl >() )
{
    impl_->widget_->installEventFilter( this );
    setWidget( impl_->widget_ );
}

Core::IDocument *
DataprocEditor::document() const
{
    return impl_->file_.get();
}

QWidget *
DataprocEditor::toolBar()
{
    // ADDEBUG() << "########### DataprocEditor::" << __FUNCTION__ << " ####################";
    return 0;
}

Core::IEditor *
DataprocEditor::duplicate()
{
    // ADDEBUG() << "########### DataprocEditor::" << __FUNCTION__ << " ####################";
    return 0;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

// QTC4, 9
bool
DataprocEditor::eventFilter( QObject * object, QEvent * event )
{
    if ( object == impl_->widget_ ) {
        if ( event->type() == QEvent::Show ) {
            if ( impl_->file_ )
                Core::ModeManager::activateMode( Utils::Id( Constants::C_DATAPROCESSOR ) );
        }
    }
    return false;
}
