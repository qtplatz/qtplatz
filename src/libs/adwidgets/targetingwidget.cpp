/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "targetingwidget.hpp"
#include "targetingform.hpp"
//#include "targetingtable.hpp"
#include "moltable.hpp"
#include "targetingadducts.hpp"
#include <adportable/is_type.hpp>
#include <adprot/digestedpeptides.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <QSplitter>
#include <QBoxLayout>

using namespace adwidgets;

TargetingWidget::TargetingWidget(QWidget *parent) : QWidget(parent)
                                                  , form_(0)
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);
        
        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( form_ = new TargetingForm ) ); 
            //splitter->addWidget( ( table_ = new TargetingTable ) );
            splitter->addWidget( new MolTable ); 
            splitter->addWidget( (new TargetingAdducts) );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );
            splitter->setStretchFactor( 2, 1 );
            splitter->setOrientation ( Qt::Horizontal );

            layout->addWidget( splitter );

        }
    }
    connect( form_, &TargetingForm::triggerProcess, [this] { emit triggerProcess( "TargetingWidget" ); } );
}

TargetingWidget::~TargetingWidget()
{
    delete form_;
}

QWidget *
TargetingWidget::create( QWidget * parent )
{
    return new TargetingWidget( parent );
}

void
TargetingWidget::OnCreate( const adportable::Configuration& )
{
}

void
TargetingWidget::OnInitialUpdate()
{
    adcontrols::TargetingMethod m; // default

    if ( auto table = findChild< MolTable *>() ) {
        table->onInitialUpdate();
        table->setContents( m.molecules() );
    }

    form_->setContents( m );
        
    if ( auto tree = findChild< TargetingAdducts * >() ) {
        tree->OnInitialUpdate();
        tree->setContents( m );
    }
}

void
TargetingWidget::onUpdate( boost::any&& )
{
}

void
TargetingWidget::OnFinalClose()
{
}

bool
TargetingWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( a ) ) {

        if ( adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( a ) ) {

            adcontrols::TargetingMethod method( adcontrols::TargetingMethod::idTargetFormula );

            form_->getContents( method );

            //if ( auto table = findChild< TargetingTable * >() )
            // table->getContents( method );
            if ( auto table = findChild< MolTable *>() )
                table->getContents( method.molecules() );

            if ( auto tree = findChild< TargetingAdducts * >() )
                tree->getContents( method );
            
            pm->appendMethod( method );

            return true;
        }
    }
    return false;
}

bool
TargetingWidget::setContents( boost::any&& a )
{
	if ( adportable::a_type< adprot::digestedPeptides >::is_a( a ) ) {

        //auto digested = boost::any_cast< adprot::digestedPeptides >( a );
        //table_->setContents( digested );
        return false;

    } else if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( a ) ) {

        const adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( a );

        if ( const adcontrols::TargetingMethod * t = pm.find< adcontrols::TargetingMethod >() ) {

            form_->setContents( *t );

            if ( auto table = findChild< MolTable *>() )
                table->setContents( t->molecules() );
        
            if ( auto tree = findChild< TargetingAdducts * >() )
            tree->setContents( *t );
        }

    }
    
    return false;
}

