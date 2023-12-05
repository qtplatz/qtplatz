/**************************************************************************
** Copyright (C) 2010-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "rxneditform.hpp"
#include "sqledit.hpp"
#include <utils/styledbar.h>
#include <adportable/debug.hpp>
#include <QBoxLayout>
#include <QCompleter>
#include <QFile>
#include <QPushButton>
#include <QStringListModel>

using chemistry::RxnEditForm;


RxnEditForm::RxnEditForm(QWidget *parent) : QWidget(parent)
{
    auto vLayout = new QVBoxLayout( this );
    vLayout->setContentsMargins( 2, 2, 2, 2 );

    if ( auto editor = new QPlainTextEdit() ) {
        editor->installEventFilter( this );
        vLayout->addWidget( editor );

        if ( auto toolBar = new Utils::StyledBar ) {
            toolBar->setProperty( "topBorder", true );
            toolBar->setSingleRow( true );
            //toolBar->setLightColored( false );
            QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
            toolBarLayout->setContentsMargins( {} );
            toolBarLayout->setSpacing( 2 );

            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );

            toolBarLayout->addWidget( new Utils::StyledSeparator );

            if ( auto button = new QPushButton( "Execute Query" ) ) {
                toolBarLayout->addWidget( button );
            }
            vLayout->addWidget( toolBar );
        }
    }
}

RxnEditForm::~RxnEditForm()
{
}

void
RxnEditForm::setSmarts( const QString& t )
{
    if ( auto textEdit = findChild< QPlainTextEdit * >() )     {
        textEdit->clear();
        textEdit->insertPlainText( t );
    }
}

QString
RxnEditForm::smarts() const
{
    if ( auto textEdit = findChild< SqlEdit * >() )
        return textEdit->toPlainText();

    return {};
}

void
RxnEditForm::setReactant( const QString& t )
{
}

QString
RxnEditForm::smiles() const
{
    return {};
}
