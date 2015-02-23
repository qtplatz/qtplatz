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
                                                          , form_( new adwidgets::CentroidForm )
{
    auto topLayout = new QHBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );
    topLayout->addStretch( 1 );

    layout_->addWidget( form_ ); 

    auto tolerances = new QWidget;
    layout_->addWidget( tolerances, 0, 1 ); // row = 0; column = 1
    auto tLayout = new QVBoxLayout( tolerances );
    // ----------------------------
    // |             | Lock mass  | PeakFind
    // |             --------------
    // |             | assign     |
    // |---------------------------
    if ( auto form = new adwidgets::MSLockForm ) {
        tLayout->addWidget( form );
        // MSLock GroupBox check state --> reflect to MS Lock row in Compounds table
        connect( form, &adwidgets::MSLockForm::toggled, this, [] ( bool checked ){ QuanDocument::instance()->mslock_enabled( checked ); } );
    }

    if ( auto form = new adwidgets::MSToleranceForm )
        tLayout->addWidget( form );

    if ( auto widget = new QWidget ) {
        layout_->addWidget( widget, 0, 2 );// row = 0, column = 2
        auto layout = new QVBoxLayout( widget );
        if ( auto form = new adwidgets::PeakMethodForm ) {
            layout->addWidget( form );
            form->OnInitialUpdate();
            // connect
        }
    }
        

    tLayout->addStretch( 1 );

    form_->OnInitialUpdate();

    connect( form_, &adwidgets::CentroidForm::valueChanged, this, &ProcessMethodWidget::commit );

    QuanDocument::instance()->register_dataChanged( [this]( int id, bool load ){ handleDataChanged( id, load ); });
    
}


void
ProcessMethodWidget::handleDataChanged( int id, bool load )
{
    if ( id == idProcMethod && load ) {
        const adcontrols::ProcessMethod pm = QuanDocument::instance()->procMethod();
        boost::any a( pm );
        form_->setContents( a );
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
    form_->getContents( pm );

    if ( auto form = findChild< adwidgets::MSLockForm * >() )
        form->getContents( pm );

    if ( auto form = findChild< adwidgets::MSToleranceForm * >() ) {
        adcontrols::TargetingMethod t;
        form->getContents( t );
        pm.appendMethod( t );
    }

    QuanDocument::instance()->setProcMethod( pm );
}
