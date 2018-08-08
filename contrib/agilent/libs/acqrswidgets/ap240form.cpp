/**************************************************************************
** Copyright (C) 2010-     Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "ap240form.hpp"
#include "acqiriswidget.hpp"
#include "ui_ap240form.h"
#include <acqrscontrols/acqiris_method.hpp>
#include <acqrscontrols/ap240/method.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/threshold_action.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <adportable/serializer.hpp>
#include <adwidgets/findslopeform.hpp>
#include <adwidgets/hostaddrdialog.hpp>
#include <adwidgets/mouserbuttonfilter.hpp>
#include <adwidgets/thresholdactionform.hpp>
#include <qtwrapper/make_widget.hpp>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QMouseEvent>
#include <QPair>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QVBoxLayout>
#include <boost/exception/all.hpp>

using namespace acqrswidgets;

ap240form::ap240form(QWidget *parent) : QWidget(parent)
                                      , ui( new Ui::ap240form )
{
    ui->setupUi(this);


    // Software TDC (Slope Time Converter) UI
    if ( auto layout = new QVBoxLayout( ui->groupBox_2 ) ) {    
        layout->setSpacing( 0 );
        layout->setMargin( 0 );
        ///////////// Slope detect ////////////////
        if ( auto tab = new QTabWidget() ) {
            layout->addWidget( tab );
            int idx = 0;
            for ( auto& title : { tr( "CH1" ), tr( "CH2" ) } ) {
                auto ch = new adwidgets::findSlopeForm();
                ch->setTitle( idx++, title );
                ch->setObjectName( title );
                tab->addTab( ch, title );

                // enable|disable
                connect( ch, &adwidgets::findSlopeForm::valueChanged, [this] ( int ch ) {
                        emit valueChanged( idSlopeTimeConverter, ch ); } );
            }

            ////////// Threshold Action ///////////
            auto form = qtwrapper::make_widget< adwidgets::ThresholdActionForm >( "ThresholdActionForm" );
            tab->addTab( form, tr( "Action" ) );
            connect( form, &adwidgets::ThresholdActionForm::valueChanged, [this] () { emit valueChanged( idThresholdAction, 0 ); } );
        }
    }

    if ( auto layout = new QVBoxLayout( ui->groupBox_3 ) ) {
        layout->setSpacing( 0 );
        layout->setMargin( 0 );
        if ( auto widget = new AcqirisWidget() ) {
            layout->addWidget( widget );
            widget->setStyleSheet( "QTreeView { background: #e8f4fc; }\n"
                                   "QTreeView::item:open { background-color: #1d3dec; color: white; }" );
            connect( widget, &AcqirisWidget::dataChanged, [&]( const AcqirisWidget * w, int cat ){
                    emit valueChanged( idAP240Any, 0 );
                });
            connect( widget, &AcqirisWidget::stateChanged, [&]( const QModelIndex& index, bool ){
                    emit valueChanged( idChannels, index.row() - 2 );
                });            
        }

        if ( auto gbx = qtwrapper::make_widget< QGroupBox >( "RemoteAccess", tr("Digitizer Remote Access") ) ) {
            
            gbx->setFlat( true );
            layout->addWidget( gbx );
            gbx->setCheckable( true );

            if ( auto hLayout = new QHBoxLayout( gbx ) ) {

                hLayout->setSpacing( 0 );
                hLayout->setMargin( 0 );

                hLayout->addWidget( qtwrapper::make_widget< QLabel >( "url", tr( "URL" ) ) );
                
                auto edit = qtwrapper::make_widget< QLineEdit >( "url" );
                hLayout->addWidget( edit );
                

                if ( ( eventFilter_ = std::make_unique< adwidgets::MouseRButtonFilter >() ) ) {
                    eventFilter_->installOn( edit );
                    connect( eventFilter_.get(), &adwidgets::MouseRButtonFilter::mouseEvent, [&]( QWidget * w, QMouseEvent * ev){
                            QMenu menu;
                            menu.addAction( tr("Edit"), [this]{
                                    adwidgets::HostAddrDialog dlg( this );
                                    dlg.setModal( true );
                                    if ( auto w = findChild< QLineEdit * >( "url" ) ) {
                                        dlg.setUrl( w->text() );
                                        if ( dlg.exec() ) {
                                            host_ = dlg.hostAddr().first;
                                            port_ = dlg.hostAddr().second;
                                            emit hostChanged( remote_, host_, port_ );
                                            if ( auto edit = qobject_cast< QLineEdit * >( w ) )
                                                edit->setText( QString( "http://%1:%2" ).arg( host_, port_ ) );
                                        }
                                    }
                                });
                            menu.exec( ev->globalPos() );
                        });
                }

                connect( gbx, &QGroupBox::toggled, this, [this,edit]( bool remote ){
                        remote_ = remote;
                        emit hostChanged( remote, host_, port_ );
                    });

                edit->setReadOnly( true );
                edit->setContextMenuPolicy( Qt::CustomContextMenu );

                setStyleSheet("QLineEdit[readOnly=\"true\"] {"
                              "color: #808080;"
                              "background-color: #F0F0F0;"
                              "border: 1px solid #B0B0B0;"
                              "border-radius: 2px;}");    
            }
        }
    }
    
    set( std::make_shared< acqrscontrols::ap240::method >() );
}

ap240form::~ap240form()
{
    delete ui;
}

// LifeCycle
void
ap240form::OnCreate( const adportable::Configuration& )
{
}

void
ap240form::OnInitialUpdate()
{
    if ( auto form = findChild< adwidgets::ThresholdActionForm * >() )
        form->OnInitialUpdate();

    // don't response to each key strokes on DoubleSpinBox
    for ( auto spin : findChildren< QDoubleSpinBox * >() )
        spin->setKeyboardTracking( false );
}

void
ap240form::OnFinalClose()
{
}

bool
ap240form::getContents( boost::any& a ) const
{
    if ( adportable::a_type< adcontrols::ControlMethodPtr >::is_a( a ) ) {

        adcontrols::ControlMethodPtr ptr = boost::any_cast<adcontrols::ControlMethodPtr>(a);        
        
        auto m = std::make_shared< acqrscontrols::ap240::method>();
        get( m );
        adcontrols::ControlMethod::MethodItem item( m->clsid(), m->modelClass() );
        item.setItemLabel( "ap240" );
        item.set<>( item, *m ); // serialize
        ptr->insert( item );
        
        return true;
        
    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_pointer( a ) ) {
        
        auto pi = boost::any_cast<adcontrols::ControlMethod::MethodItem *>( a );
        auto m = std::make_shared< acqrscontrols::ap240::method>();
        get( m );
        pi->setModelname( "ap240" );
        pi->setItemLabel( "ap240" );
        pi->set<>( *pi, *m ); // serialize
        return true;
    } else if ( adportable::a_type< acqrscontrols::ap240::method >::is_pointer( a ) ) {
        assert( 0 );
    }
    return false;
}

bool
ap240form::setContents( boost::any&& a )
{
    const adcontrols::ControlMethod::MethodItem * pi(0);

    if ( adportable::a_type < std::shared_ptr< const adcontrols::ControlMethod::Method > >::is_a( a ) ) {
        // adcontrols::ControlMethod::Method
        // find first one
        if ( auto ptr = boost::any_cast<std::shared_ptr< const adcontrols::ControlMethod::Method>>( a ) ) {
            auto it = ptr->find( ptr->begin(), ptr->end(), acqrscontrols::ap240::method::clsid() );
            if ( it != ptr->end() )
                pi = &( *it );
        }

    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_pointer( a ) ) {

        pi = boost::any_cast<const adcontrols::ControlMethod::MethodItem * >( a );             

    } else if ( adportable::a_type< adcontrols::ControlMethod::MethodItem >::is_a( a ) ) {   

        pi = &boost::any_cast<const adcontrols::ControlMethod::MethodItem& >( a );
    }

    if ( pi ) {
        auto m = std::make_shared< acqrscontrols::ap240::method >();
		try {
            pi->get<>( *pi, *m );
            set( m );
		} catch (boost::exception& ex) {
			QMessageBox::warning(this, "AP240 Method", QString::fromStdString(boost::diagnostic_information(ex)));
		} catch ( ... ) {
			QMessageBox::warning(this, "AP240 Method", QString::fromStdString(boost::current_exception_diagnostic_information()));
		}

    }
    return true;
}

void
ap240form::onInitialUpdate()
{
}

void
ap240form::onStatus( int )
{
}

void
ap240form::get( std::shared_ptr< acqrscontrols::ap240::method > m ) const
{
    if ( auto w = findChild< AcqirisWidget * >() ) {
        w->getContents( m );
        get( 0, m->slope1_ );
        get( 1, m->slope2_ );
        get( m->action_ );
    }
}

void
ap240form::set( std::shared_ptr< const acqrscontrols::ap240::method> m )
{
    if ( auto w = findChild< AcqirisWidget * >() ) {
        w->setContents( m );
        set( 0, m->slope1_ );
        set( 1, m->slope2_ );
        set( m->action_ );
    }    
}

void
ap240form::get( int ch, adcontrols::threshold_method& m ) const
{
    const QString names[] = { "CH1", "CH2" };

    if ( auto form = findChild< adwidgets::findSlopeForm * >( names[ ch ] ) ) {
        form->get( m );
    }
}

void
ap240form::set( int ch, const adcontrols::threshold_method& m )
{
    const QString names[] = { "CH1", "CH2" };

    if ( auto form = findChild< adwidgets::findSlopeForm * >( names[ ch ] ) ) {
        form->set( m );
    }
}

void
ap240form::get( adcontrols::threshold_action& m ) const
{
    if ( auto form = findChild< adwidgets::ThresholdActionForm *>() )
        form->get( m );
}

void
ap240form::set( const adcontrols::threshold_action& m )
{
    if ( auto form = findChild< adwidgets::ThresholdActionForm *>() )
        form->set( m );
}

void
ap240form::setRemoteAccess( bool remote, const QString& host, const QString& port )
{
    remote_ = remote;
    host_ = host;
    port_ = port;
    
    if ( auto gbx = findChild< QGroupBox * >( "RemoteAccess" ) ) {
        gbx->setChecked( remote );
        if ( auto edit = gbx->findChild< QLineEdit * >() )
            edit->setText( QString( "http://%1:%2" ).arg( host, port ) );
    }
}

bool
ap240form::remoteAccess( QString& host, QString& port ) const
{
    bool remote( false );
    QString rhost;

    if ( auto gbx = findChild< QGroupBox * >( "RemoteAccess" ) ) {
        remote = gbx->isChecked();
        host = host_;
        port = port_;
    }
    return remote;
}

