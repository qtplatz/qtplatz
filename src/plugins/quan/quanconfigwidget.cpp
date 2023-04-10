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

#include "paneldata.hpp"
#include "quanconfigform.hpp"
#include "quanconfigwidget.hpp"
#include "quanconstants.hpp"
#include "document.hpp"
#include "samplemethodform.hpp"
#include <adcontrols/processmethod.hpp>
#include <adportable/profile.hpp>
#include <qtwrapper/make_widget.hpp>
#include <utils/styledbar.h>
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
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setContentsMargins( {} );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    if ( auto toolBar = new Utils::StyledBar ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setContentsMargins( {} );
        toolBarLayout->setSpacing( 0 );
        auto label = new QLabel;
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

    const int row = layout_->rowCount();

    if ( auto form = qtwrapper::make_widget< QuanConfigForm >( "quanConfigForm" ) ) {
        layout_->addWidget( form, row + 1, 0 );
    }

    if ( auto sform = qtwrapper::make_widget< quan::SampleMethodForm >( "sampleMethod" ) ) {
        layout_->addWidget( sform, row, 0 );
    }

    if ( auto sform = findChild< SampleMethodForm * >() ) {
        if ( auto qform = findChild< QuanConfigForm * >() ) {

            if ( auto qm = document::instance()->getm< adcontrols::QuanMethod >() ) {
                sform->setSelection( qm->inlet() );
                qform->setContents( *qm );
            } else {
                commit();
            }

            connect( qform->spinLevels(), static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged)
                     , this, [this] ( int value ){ emit onLevelChanged( value ); } );

            connect( qform->spinReplicates(), static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged)
                     , this, [this] ( int value ){ emit onReplicatesChanged( value ); } );

            connect( sform, &SampleMethodForm::onSampleMethodChanged, qform, &QuanConfigForm::handleInletChanged);
#if __cplusplus >= 201703L
            connect( sform, &SampleMethodForm::onSampleMethodChanged, [this] ( auto t ) { emit onSampleInletChanged( t ); } );
#else
            // support for c++14
            connect( sform, &SampleMethodForm::onSampleMethodChanged, [this] ( adcontrols::Quan::QuanInlet t ) { emit onSampleInletChanged( t ); } );
#endif
        }
    }

    document::instance()->connectDataChanged( [this]( int id, bool fnChanged ){ handleDataChanged( id, fnChanged ); });
    // setStyleSheet( "QuanConfigForm { font-size: 10pt }" );
}

void
QuanConfigWidget::commit()
{
    if ( auto form = findChild< QuanConfigForm * >() ) {
        adcontrols::QuanMethod m;
        form->getContents( m );
        adcontrols::Quan::QuanInlet inlet{ adcontrols::Quan::Chromatography };
        if ( auto sform = findChild< SampleMethodForm * >() )
            inlet = sform->currSelection();
        m.setInlet( inlet );
        document::instance()->setm( m );
    }
}

void
QuanConfigWidget::handleDataChanged( int id, bool )
{
    if ( auto form = findChild< QuanConfigForm * >() ) {
        if ( auto qm = document::instance()->getm< adcontrols::QuanMethod >() ) {
            form->setContents( *qm );
            if ( auto sform = findChild< SampleMethodForm * >() ) {
                sform->setSelection( qm->inlet() );
            }
        }
    }
}

void
QuanConfigWidget::importQuanMethod()
{
    QString name = QFileDialog::getOpenFileName( this
                                                 , tr( "Open Quan Method..." )
                                                 , document::instance()->lastMethodDir()
                                                 , tr( "Quan Method Files(*.qmth);;XML Files(*.xml)" ) );
    if ( !name.isEmpty() ) {
        adcontrols::ProcessMethod m;
        document::instance()->load( name.toStdWString(), m, false );
        document::instance()->replace_method( m );
    }
}
