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
#include <adcontrols/controlmethod.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adplugin/lifecycle.hpp>
#include <boost/any.hpp>
#include <QVBoxLayout>
#include <QSplitter>
#include <QMenu>

namespace adwidgets {

    class ControlMethodWidget::impl {
        impl( const impl& ) = delete;
    public:
        ControlMethodTable * table_;
        QTabWidget * tab_;
        std::shared_ptr< adcontrols::ControlMethod > method_;
        QList< QWidget * > widgets_;
        std::vector< adplugin::LifeCycle * > editors_;

        impl() : table_( new ControlMethodTable )
               , tab_( new QTabWidget )
               , method_( std::make_shared< adcontrols::ControlMethod >() ) {
        }

    };

}

using namespace adwidgets;

ControlMethodWidget::~ControlMethodWidget()
{
}

ControlMethodWidget::ControlMethodWidget(QWidget *parent) : QWidget(parent)
                                                          , impl_( new impl() )
{
    if ( auto layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(0);
        layout->addWidget( impl_->tab_ );
        impl_->tab_->addTab( impl_->table_, "Time Events" );
        impl_->tab_->setTabPosition( QTabWidget::South );

        QSizePolicy sizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth( false ); 
        impl_->table_->setSizePolicy( sizePolicy );
    }

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QWidget::customContextMenuRequested, this, &ControlMethodWidget::showContextMenu);
    connect( impl_->table_, &QTableView::customContextMenuRequested, this, &ControlMethodWidget::showContextMenu);
}

QSize
ControlMethodWidget::sizeHint() const
{
	return QSize( 200, 100 );
}

void
ControlMethodWidget::addWidget( QWidget * widget, const QString& label )
{
    impl_->tab_->addTab( widget, label );
}

void
ControlMethodWidget::addWidget( QWidget * widget, const QIcon& icon, const QString& label )
{
    impl_->tab_->addTab( widget, icon, label );
}

void
ControlMethodWidget::addItem( const QString& title, QWidget * widget )
{
    impl_->table_->addItem( title );
    impl_->widgets_.push_back( widget );

    adplugin::LifeCycleAccessor accessor( widget );
    if ( auto lifecycle = accessor.get() ) {

        lifecycle->OnInitialUpdate();

        boost::any a( impl_->method_ );
        lifecycle->getContents( a );
        impl_->editors_.push_back( lifecycle );

    }
    impl_->table_->setContents( *impl_->method_ );
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
    if ( !adportable::a_type< adcontrols::ControlMethod >::is_a( any ) )
        return false;

    const adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( any );
    const adcontrols::CentroidMethod * t = pm.find< adcontrols::CentroidMethod >();
    if ( ! t )
        return false;
    *pMethod_ = *t;
    // update_data( *pMethod_ );
#endif
    return true;
}

void
ControlMethodWidget::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = this;
}

void
ControlMethodWidget::showContextMenu( const QPoint& pt )
{
    impl_->table_->showContextMenu( pt );
}
