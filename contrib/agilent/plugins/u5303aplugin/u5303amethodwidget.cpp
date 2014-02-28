/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "u5303amethodwidget.hpp"
#include "u5303aform.hpp"
#include "u5303amethodtable.hpp"
#include "document.hpp"
#include <adinterface/controlserver.hpp>
#include <u5303a/digitizer.hpp>
#include <QSplitter>
#include <QBoxLayout>

using namespace u5303a;

u5303AMethodWidget::u5303AMethodWidget(QWidget *parent) : QWidget(parent)
{
    if ( QSplitter * splitter = new QSplitter ) {
        splitter->addWidget( new u5303AForm(this) );
        splitter->addWidget( new u5303AMethodTable(this) );
        splitter->setOrientation( Qt::Horizontal );
        splitter->setStretchFactor( 0, 1 );
        splitter->setStretchFactor( 1, 4 );

        if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {
            layout->setMargin( 0 );
            layout->setSpacing( 0 );
            layout->addWidget( splitter );
        }
    }
}

void
u5303AMethodWidget::onInitialUpdate()
{
    if ( auto form = findChild< u5303AForm * >() ) {
        form->onInitialUpdate();
        connect( form, SIGNAL( on_trigger_apply() ), this, SLOT( handle_trigger_apply() ) );
    }
    if ( auto table = findChild< u5303AMethodTable * >() ) {
        table->onInitialUpdate();
        table->setContents( document::instance()->method() );
    }
}

void
u5303AMethodWidget::onStatus( int st )
{
    if ( auto form = findChild< u5303AForm * >() )
        form->onStatus( st );        
}

void
u5303AMethodWidget::handle_trigger_apply()
{
    if ( auto table = findChild< u5303AMethodTable * >() ) {
        u5303a::method m;
        if ( table->getContents( m ) ) {
            document::instance()->prepare_for_run( m );
        }
    }
}


