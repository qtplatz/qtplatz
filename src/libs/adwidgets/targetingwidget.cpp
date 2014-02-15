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

#include "targetingwidget.hpp"
#include "targetingform.hpp"
#include "targetingtable.hpp"
#include <adportable/is_type.hpp>
#include <adprot/digestedpeptides.hpp>
#include <QSplitter>
#include <QBoxLayout>

using namespace adwidgets;

TargetingWidget::TargetingWidget(QWidget *parent) : QWidget(parent)
                                                  , table_(0)
                                                  , form_(0)
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);
        
        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( form_ = new TargetingForm ) ); 
            splitter->addWidget( ( table_ = new TargetingTable ) ); 
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 1 );
            splitter->setOrientation ( Qt::Horizontal );

            layout->addWidget( splitter );
        }
    }

}

TargetingWidget::~TargetingWidget()
{
    delete table_;
    delete form_;
}

QWidget *
TargetingWidget::create( QWidget * parent )
{
    return new TargetingWidget( parent );
}

// void *
// TargetingWidget::query_interface_workaround( const char * typenam )
// {
//     if ( typenam == typeid( TargetingWidget ).name() )
//         return static_cast< TargetingWidget * >(this);
//     return 0;
// }

void
TargetingWidget::OnCreate( const adportable::Configuration& )
{
}

void
TargetingWidget::OnInitialUpdate()
{
}

void
TargetingWidget::onUpdate( boost::any& )
{
}

void
TargetingWidget::OnFinalClose()
{
}

bool
TargetingWidget::getContents( boost::any& ) const
{
    return false;
}

bool
TargetingWidget::setContents( boost::any& a )
{
	if ( adportable::a_type< adprot::digestedPeptides >::is_a( a ) ) {
        auto digested = boost::any_cast< adprot::digestedPeptides >( a );
		table_->setContents( digested );
        
        return true;
    }
    return false;
}

