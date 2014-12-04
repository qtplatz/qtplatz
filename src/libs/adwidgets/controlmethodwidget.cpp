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
#include <adportable/debug.hpp>
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
        std::vector< std::pair< adplugin::LifeCycle *, QWidget * > > editors_;
        ControlMethodWidget * this_;

        impl( ControlMethodWidget * pThis ) : table_( new ControlMethodTable( pThis ) )
                                            , tab_( new QTabWidget )
                                            , method_( std::make_shared< adcontrols::ControlMethod >() )
                                            , this_( pThis ){
        }

        void addMethod( const QString& key ) {
            auto it = std::find_if( editors_.begin(), editors_.end(), [key] ( std::pair<adplugin::LifeCycle*, QWidget *>& e ){
                    return key == e.second->objectName();
                } );
            if ( it != editors_.end() ) {
                if ( auto editor = it->first ) {
                    adcontrols::controlmethod::MethodItem mi;
                    boost::any a( &mi );
                    editor->getContents( a );
                    mi.isInitialCondition( false );
                    mi.time( 0 );
                    table_->append( mi );
                }
            }
        }

        bool getMatchedMethod( adcontrols::controlmethod::MethodItem& mi, const adcontrols::controlmethod::MethodItem& curr ) {
            boost::any a( &mi );
            for ( auto editor : editors_ ) {
                if ( editor.first->getContents( a ) ) {
                    if ( mi.modelname() == curr.modelname() && mi.itemLabel() == curr.itemLabel() )
                        return true;
                }
            }
            return false;
        }

        bool setMatchedMethod( const adcontrols::controlmethod::MethodItem& curr ) {
            adcontrols::controlmethod::MethodItem mi( curr );
            boost::any a( &mi );
            for ( auto editor : editors_ ) {
                if ( editor.first->getContents( a ) ) {

                    if ( mi.modelname() == curr.modelname() && mi.itemLabel() == curr.itemLabel() ) {

                        boost::any a( curr );
                        editor.first->setContents( a );
                        emit this_->onCurrentChanged( editor.second );
                        return true;

                    }
                }
            }
            return false;
        }
    };

}

using namespace adwidgets;

ControlMethodWidget::~ControlMethodWidget()
{
}

ControlMethodWidget::ControlMethodWidget(QWidget *parent) : QWidget(parent)
                                                          , impl_( new impl(this) )
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
    connect( impl_->table_, &ControlMethodTable::onAddMethod, [this] ( const QString& item ){ impl_->addMethod( item ); } );
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
ControlMethodWidget::addEditor( QWidget * widget )
{
    impl_->table_->addItem( widget->objectName() ); // for menu

    adplugin::LifeCycleAccessor accessor( widget );
    if ( auto lifecycle = accessor.get() ) {

        lifecycle->OnInitialUpdate();

        boost::any a( impl_->method_ );
        lifecycle->getContents( a );
        impl_->editors_.push_back( std::make_pair( lifecycle, widget ) );

    }
    impl_->table_->setSharedPointer( impl_->method_ );
}

bool
ControlMethodWidget::getMethod( adcontrols::controlmethod::MethodItem& mi )
{
    adcontrols::controlmethod::MethodItem temp( mi );
    if ( impl_->getMatchedMethod( temp, mi ) ) {
        mi = temp;
        return true;
    }
    return false;
}

bool
ControlMethodWidget::setMethod( const adcontrols::controlmethod::MethodItem& mi )
{
    adcontrols::controlmethod::MethodItem temp( mi );
    if ( impl_->setMatchedMethod( mi ) ) {
        return true;
    }
    return false;

}

void
ControlMethodWidget::setControlMethod( const adcontrols::ControlMethod& m )
{
    impl_->method_ = std::make_shared< adcontrols::ControlMethod >( m );
    impl_->table_->setSharedPointer( impl_->method_ );
}

bool
ControlMethodWidget::getControlMethod( adcontrols::ControlMethod& m )
{
    impl_->table_->commit();
    m = *(impl_->method_);
    return true;
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
ControlMethodWidget::getContents( boost::any& ) const
{
    return true;
}

bool
ControlMethodWidget::setContents( boost::any& )
{
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
