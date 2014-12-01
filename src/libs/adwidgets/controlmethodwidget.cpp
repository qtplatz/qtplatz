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

#include "controlmethodwidget.hpp"
#include "controlmethodtable.hpp"
#include <QVBoxLayout>
#include <QSplitter>

using namespace adwidgets;

ControlMethodWidget::ControlMethodWidget(QWidget *parent) : QWidget(parent)
                                                          , table_( new ControlMethodTable )
                                                          , tab_( new QTabWidget )
{
    if ( auto layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(0);
        layout->addWidget( tab_ );
        tab_->addTab( table_, "Time Events" );
		tab_->setTabPosition( QTabWidget::South );

        QSizePolicy sizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth( false ); 
        table_->setSizePolicy(sizePolicy);
    }
}

QSize
ControlMethodWidget::sizeHint() const
{
	return QSize( 200, 100 );
}

void
ControlMethodWidget::addWidget( QWidget * widget, const QString& label )
{
	tab_->addTab( widget, label );
}

void
ControlMethodWidget::addWidget( QWidget * widget, const QIcon& icon, const QString& label )
{
	tab_->addTab( widget, icon, label );
}

void
ControlMethodWidget::handleAdd( const adcontrols::controlmethod::MethodItem& )
{
}

////////////////
void
ControlMethodWidget::OnCreate( const adportable::Configuration& )
{
}

void
ControlMethodWidget::OnInitialUpdate()
{
    //update_data( *pMethod_ );
}

void
ControlMethodWidget::OnFinalClose()
{
}

bool
ControlMethodWidget::getContents( boost::any& any ) const
{
#if 0
    if ( ! adportable::a_type< adcontrols::ProcessMethod >::is_pointer( any ) )
        return false;

    adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any );
    const_cast< ControlMethodWidget *>(this)->update_data();
    pm->appendMethod< adcontrols::CentroidMethod >( *pMethod_ );
#endif    
    return true;
}

bool
ControlMethodWidget::setContents( boost::any& any )
{
#if 0
    if ( ! adportable::a_type< adcontrols::ProcessMethod >::is_a( any ) )
        return false;

    const adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( any );
    const adcontrols::CentroidMethod * t = pm.find< adcontrols::CentroidMethod >();
    if ( ! t )
        return false;
    *pMethod_ = *t;
    update_data( *pMethod_ );
#endif
    return true;
}

void
ControlMethodWidget::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = this;
}
