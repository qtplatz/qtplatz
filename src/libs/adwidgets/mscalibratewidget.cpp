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

#include "mscalibratewidget.hpp"
#include "mscalibrateform.hpp"
#include "msreferencetable.hpp"
#include "msreference.hpp"
#include <adcontrols/processmethod.hpp>
#include <adportable/is_type.hpp>
#include <adportable/debug.hpp>
#include <QVBoxLayout>
#include <QWidget>
#include <QSplitter>

using namespace adwidgets;

MSCalibrateWidget::MSCalibrateWidget(QWidget *parent) :  QWidget(parent)
                                                      , form_( new MSCalibrateForm )
                                                      , table_( new MSReferenceTable )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);
        
        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( form_ );
            splitter->addWidget( table_ );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );

            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
    connect( form_, &MSCalibrateForm::triggerProcess, [this] { emit triggerProcess( "MSCalibrateWidget" ); } );
}

void
MSCalibrateWidget::OnCreate( const adportable::Configuration& )
{
}

void
MSCalibrateWidget::OnInitialUpdate()
{
    table_->onInitialUpdate();
    connect( form_, &MSCalibrateForm::addReference, table_, &MSReferenceTable::handleAddReference );
}

void
MSCalibrateWidget::onUpdate( boost::any& )
{
}

void
MSCalibrateWidget::OnFinalClose()
{
    // ADDEBUG() << "MSCalibrateWidget OnFinalClose";
    form_->finalClose();
}

bool
MSCalibrateWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( a ) ) {

        if ( adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( a ) ) {

            adcontrols::MSCalibrateMethod method;

            form_->getContents( method );
            table_->getContents( method );

            pm->appendMethod( method );

            return true;
        }
    }

    return false;
}

bool
MSCalibrateWidget::setContents( boost::any& a )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( a ) ) {

        const adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( a );

        if ( const adcontrols::MSCalibrateMethod * t = pm.find< adcontrols::MSCalibrateMethod >() ) {
            form_->setContents( *t );
            table_->setContents( *t );
            return true;
        }

    }
    return false;
}
