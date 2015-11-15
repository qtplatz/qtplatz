/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "tofchromatogramswidget.hpp"
#include "tofchromatogramsform.hpp"
#include "moltable.hpp"
#include <adportable/is_type.hpp>
#include <adcontrols/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod.hpp>
#include <QSplitter>
#include <QBoxLayout>
#include <QMenu>

using namespace adwidgets;

TofChromatogramsWidget::TofChromatogramsWidget(QWidget *parent) : QWidget(parent)
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);
        
        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new TofChromatogramsForm ) ); 
            splitter->addWidget( ( new MolTable ) );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
//    if ( auto form = findChild< MSSimulatorForm * >() )
//        connect( form, &MSSimulatorForm::triggerProcess, [this] { run(); } );
}

TofChromatogramsWidget::~TofChromatogramsWidget()
{
}

void
TofChromatogramsWidget::OnCreate( const adportable::Configuration& )
{
}

void
TofChromatogramsWidget::OnInitialUpdate()
{
    if ( auto form = findChild< TofChromatogramsForm * >() )
        form->OnInitialUpdate();
    
    if ( auto table = findChild< MolTable *>() ) {
        table->onInitialUpdate();
        connect( table, &MolTable::onContextMenu, this, &TofChromatogramsWidget::handleContextMenu );
    }
}

void
TofChromatogramsWidget::onUpdate( boost::any& )
{
}

void
TofChromatogramsWidget::OnFinalClose()
{
}

bool
TofChromatogramsWidget::getContents( boost::any& a ) const
{
    return false;
}

bool
TofChromatogramsWidget::setContents( boost::any& a )
{
    return false;
}

#if 0
void
TofChromatogramsWidget::setTimeSquaredScanLaw( double flength, double acceleratorVoltage, double tdelay )
{
    if ( auto form = findChild< MSSimulatorForm * >() ) {
        adcontrols::MSSimulatorMethod m;
        form->getContents( m );
        m.setLength( flength );
        m.setAcceleratorVoltage( acceleratorVoltage );
        m.setTDelay( tdelay );
        form->setContents( m );
    }
}
#endif

void
TofChromatogramsWidget::handleContextMenu( QMenu& menu, const QPoint& pt )
{
    menu.addAction( "Simulate MS Spectrum", this, SLOT( run() ) );
}


