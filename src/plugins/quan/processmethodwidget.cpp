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

#include "processmethodwidget.hpp"
#include "quandocument.hpp"
#include <adcontrols/processmethod.hpp>
#include <adwidgets/centroidform.hpp>
#include <adwidgets/mstoleranceform.hpp>
#include <adwidgets/mslockform.hpp>
#include <adwidgets/peakmethodform.hpp>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <boost/any.hpp>

using namespace quan;

ProcessMethodWidget::~ProcessMethodWidget()
{
}

ProcessMethodWidget::ProcessMethodWidget(QWidget *parent) :  QWidget(parent)
                                                          , layout_( new QGridLayout )
{
    auto topLayout = new QHBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );
    topLayout->addStretch( 1 );

    // ----------------------------------------
    // |             | Lock mass  | PeakFind  |
    // |             -------------|           |
    // |             | assign     |           |
    // ----------------------------------------
    auto centroidform = new adwidgets::CentroidForm;
    layout_->addWidget( centroidform );

    if ( auto widget = new QWidget ) { // Lock mass, assign method column
        layout_->addWidget( widget, 0, 1 ); // row = 0; column = 1
        auto tLayout = new QVBoxLayout( widget );
        if ( auto form = new adwidgets::MSLockForm ) {
            tLayout->addWidget( form );
            // MSLock GroupBox check state --> reflect to MS Lock row in Compounds table
            connect( form, &adwidgets::MSLockForm::toggled, this, [] ( bool checked ) { QuanDocument::instance()->mslock_enabled( checked ); } );
        }

        // row = 1, column 1
        if ( auto form = new adwidgets::MSToleranceForm )
            tLayout->addWidget( form );
    }

    auto peakmethodform = new adwidgets::PeakMethodForm;
    layout_->addWidget( peakmethodform, 0, 2 ); // row = 0, column = 2; Chromatography peak method column
        
    //tLayout->addStretch( 1 );
    centroidform->OnInitialUpdate();
    peakmethodform->OnInitialUpdate();

    connect( centroidform, &adwidgets::CentroidForm::valueChanged, this, &ProcessMethodWidget::commit );
    connect( peakmethodform, &adwidgets::PeakMethodForm::valueChanged, this, &ProcessMethodWidget::commit );

    QuanDocument::instance()->connectDataChanged( [this]( int id, bool load ){ handleDataChanged( id, load ); });
    
}


void
ProcessMethodWidget::handleDataChanged( int id, bool load )
{
    if ( id == idProcMethod && load ) {

        const adcontrols::ProcessMethod pm = QuanDocument::instance()->procMethod();
        //boost::any a( pm );

        if ( auto centroidform = findChild< adwidgets::CentroidForm * >() ) {
            centroidform->setContents( boost::any(pm) );
        }

        if ( auto peakmethodform = findChild< adwidgets::PeakMethodForm * >() ) {
            peakmethodform->setContents( boost::any(pm) );
        }
        
        if ( auto form = findChild< adwidgets::MSLockForm * >() ) {
            form->setContents( pm, true );
        }

        if ( auto form = findChild< adwidgets::MSToleranceForm * >() ) {
            if ( auto pTgt = pm.find< adcontrols::TargetingMethod >() ) {
                form->setContents( *pTgt );
            }
        }
    }
}

void
ProcessMethodWidget::commit()
{
    adcontrols::ProcessMethod pm;

    if ( auto form = findChild< adwidgets::CentroidForm * >() ) 
        form->getContents( pm );

    if ( auto form = findChild< adwidgets::PeakMethodForm * >() ) 
        form->getContents( pm );

    if ( auto form = findChild< adwidgets::MSLockForm * >() )
        form->getContents( pm );

    if ( auto form = findChild< adwidgets::MSToleranceForm * >() ) {
        adcontrols::TargetingMethod t;
        form->getContents( t );
        pm.appendMethod( t );
    }

    QuanDocument::instance()->setProcMethod( pm );
}
