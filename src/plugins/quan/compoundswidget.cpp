/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "compoundswidget.hpp"
#include "compoundstable.hpp"
#include "document.hpp"
#include "quanconstants.hpp"
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adportable/profile.hpp>
#include <adlog/logger.hpp>
#include <utils/styledbar.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <QFileDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QMessageBox>
#include <boost/exception/all.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

using namespace quan;

CompoundsWidget::~CompoundsWidget()
{
}

CompoundsWidget::CompoundsWidget(QWidget *parent) : QWidget(parent)
                                                  , layout_( new QGridLayout )
                                                  , table_( new CompoundsTable )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    document::instance()->connectDataChanged( [this] ( int id, bool f ){ handleDataChanged( id, f ); } );

    if ( auto toolBar = new Utils::StyledBar ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        if ( auto btnOpen = new QToolButton ) {
            btnOpen->setIcon( QIcon( ":/quan/images/fileopen.png" ) );
            btnOpen->setToolTip( tr( "Import Compounds..." ) );
            toolBarLayout->addWidget( btnOpen );
            connect( btnOpen, &QToolButton::clicked, this, [this](bool){ importCompounds(); } );
        }
        if ( auto btnSave = new QToolButton ) {
            btnSave->setDefaultAction( Core::ActionManager::instance()->command( Constants::QUAN_METHOD_SAVE )->action() );
            btnSave->setToolTip( tr( "Save Quan Method..." ) );
            toolBarLayout->addWidget( btnSave );
        }
        if ( auto edit = new QLineEdit ) {
            edit->setObjectName( Constants::editQuanMethodName );
            toolBarLayout->addWidget( edit );
            toolBarLayout->addWidget( new Utils::StyledSeparator );
        }
        layout_->addWidget( toolBar );
    }
    const int row = layout_->rowCount();
    layout_->addWidget( table_.get(), row, 0 );
}


void
CompoundsWidget::commit()
{
    adcontrols::QuanCompounds c; // uuid is bing updated.
    table_->getContents( c );
    document::instance()->setm( c );
}

void
CompoundsWidget::handleDataChanged( int id, bool )
{
    if ( id == idQuanMethod ) {
        if ( auto qm = document::instance()->getm< adcontrols::QuanMethod >() )
            table_->handleQuanMethod( *qm );
    } else if ( id == idQuanCompounds ) {
        if ( auto qc = document::instance()->getm< adcontrols::QuanCompounds >() )
            table_->setContents( *qc );
        else
            commit();
    }
}

void
CompoundsWidget::importCompounds()
{
    QString name = QFileDialog::getOpenFileName( this
                                                 , tr( "Open Quan Method..." )
                                                 , document::instance()->lastMethodDir()
                                                 , tr( "Quan Method Files(*.qmth);;XML Files(*.xml)" ) );
    if ( !name.isEmpty() ) {
        adcontrols::ProcessMethod m;
        if ( document::instance()->load( name.toStdWString(), m, false ) ) {
            if ( auto ptr = m.find< adcontrols::QuanCompounds >() ) {
                document::instance()->replace_method( *ptr );
            }
        }
    }
}
