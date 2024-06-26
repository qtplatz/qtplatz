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

#include "queryeditor.hpp"
#include "querydocument.hpp"
#include "queryconstants.hpp"
#include <coreplugin/modemanager.h>
#include <QWidget>
#include <QEvent>

namespace query {

    class QueryEditor::impl {
    public:
        impl() : file_( std::make_unique< QueryDocument >() ) {}
        ~impl() {}
        QWidget * widget_;
        std::unique_ptr< QueryDocument > file_;
    };

}

using namespace query;

QueryEditor::~QueryEditor()
{
}

QueryEditor::QueryEditor( QObject * parent ) : impl_( std::make_unique< impl >() )
{
    impl_->widget_ = new QWidget;
    // widget_->installEventFilter( this );
    setWidget( impl_->widget_ );
}

Core::IDocument *
QueryEditor::document() const
{
    return impl_->file_.get();
}


QWidget *
QueryEditor::toolBar()
{
    return 0;
}

Core::IEditor *
QueryEditor::duplicate()
{
    return 0;
}
