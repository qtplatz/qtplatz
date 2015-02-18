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

#include "quanconfigwidget.hpp"
#include "quanconfigform.hpp"
#include "quandocument.hpp"
#include "quanconstants.hpp"
#include "paneldata.hpp"
#include <utils/styledbar.h>
#include <adcontrols/processmethod.hpp>
#include <adportable/profile.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QToolButton>

#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <fstream>

using namespace quan;

QuanConfigWidget::~QuanConfigWidget()
{
}

QuanConfigWidget::QuanConfigWidget(QWidget *parent) : QWidget(parent)
                                                    , layout_( new QGridLayout )
                                                    , form_( new QuanConfigForm )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    connect( form_->spinLevels(), static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this] ( int value ){ emit onLevelChanged( value ); } );
    connect( form_->spinReplicates(), static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this] ( int value ){ emit onReplicatesChanged( value ); } );

    if ( auto toolBar = new Utils::StyledBar ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        auto label = new QLabel;
        // label->setStyleSheet( "QLabel { color : blue; }" );
        label->setText( "Configuration" );
        toolBarLayout->addWidget( label );

        if ( auto btnOpen = new QToolButton ) {
            btnOpen->setIcon( QIcon( ":/quan/images/fileopen.png" ) );
            btnOpen->setToolTip( tr( "Import configuration..." ) );
            toolBarLayout->addWidget( btnOpen );
            connect( btnOpen, &QToolButton::clicked, this, [this](bool){ importQuanMethod(); } );
        }

        if ( auto btnSave = new QToolButton ) {
            btnSave->setDefaultAction( Core::ActionManager::instance()->command( Constants::QUAN_METHOD_SAVE )->action() );
            btnSave->setToolTip( tr( "Save Quan Method..." ) );
            toolBarLayout->addWidget( btnSave );
        }
        
        auto edit = new QLineEdit;
        edit->setObjectName( Constants::editQuanMethodName );
        toolBarLayout->addWidget( edit );
        layout_->addWidget( toolBar );            
    }

    QuanDocument::instance()->register_dataChanged( [this]( int id, bool fnChanged ){ handleDataChanged( id, fnChanged ); });
    const int row = layout_->rowCount();
    layout_->addWidget( form_.get(), row, 0 );
    form_->setContents( QuanDocument::instance()->quanMethod() );
    connect( form_.get(), &QuanConfigForm::onSampleInletChanged, [this] ( int t ) { emit onSampleInletChanged( t ); } );

}

void
QuanConfigWidget::commit()
{
    adcontrols::QuanMethod m;
    form_->getContents( m );
    QuanDocument::instance()->quanMethod( m );
}

void
QuanConfigWidget::handleDataChanged( int id, bool )
{
    if ( id == idQuanMethod ) 
        form_->setContents( QuanDocument::instance()->quanMethod() );
}

void
QuanConfigWidget::importQuanMethod()
{
    QString name = QFileDialog::getOpenFileName( this
                                                 , tr( "Open Quan Method..." )
                                                 , QuanDocument::instance()->lastMethodDir()
                                                 , tr( "Quan Method Files(*.qmth);;XML Files(*.xml)" ) );
    if ( !name.isEmpty() ) {
        adcontrols::ProcessMethod m;
        QuanDocument::load( name.toStdWString(), m );
        QuanDocument::instance()->replace_method( m );
        //if ( auto ptr = m.find< adcontrols::QuanMethod >() )
        //QuanDocument::instance()->replace_method( *ptr );
    }
}
