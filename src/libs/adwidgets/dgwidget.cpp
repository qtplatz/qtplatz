/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "dgwidget.hpp"
#include "dgprotocolform.hpp"
#include <adplugin/lifecycle.hpp>
#include <adplugin_manager/lifecycle.hpp>
#include <adportable/debug.hpp>
#include <qtwrapper/make_widget.hpp>
#include <QBoxLayout>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QTabWidget>
#include <QtGlobal>
#include <memory>
#include <mutex>

using namespace adwidgets;

dgWidget::dgWidget( QWidget * parent ) : QFrame( parent )
{
    using namespace qtwrapper;

    resize( 200, 200 );
    
    if ( auto layout = new QVBoxLayout( this ) ) {

        if ( auto gridLayout = new QGridLayout() ) {

            gridLayout->setVerticalSpacing( 0 );
            
            const int row = 0;
            int col = 0;

            // url
            gridLayout->addWidget( new QLabel( "URL" ), row, col++ );
            gridLayout->addWidget( make_widget< QLineEdit >( "url" ), row, col++ );

            // interval
            gridLayout->addWidget( new QLabel( "Interval (ms)" ), row, col++ );
            if ( auto spin = make_widget< QDoubleSpinBox >("interval") ) {
            	spin->setMaximum( 10000.0 ); // 10s
            	spin->setMinimum( 0.0 );
            	gridLayout->addWidget( spin, row, col++ );
            }

            //tick
            gridLayout->addWidget( new QLabel( "Tick" ), row, col++ );
            gridLayout->addWidget( make_widget< QLineEdit >( "eventText", "tick" ), row, col++ );

            gridLayout->setColumnStretch(1,2);

            layout->addLayout( gridLayout );
        }
        
        if ( auto tab = new QTabWidget() ) {
            tab->addTab( new dgProtocolForm(), "Proto1" );
            layout->addWidget( tab );
            tab->setStyleSheet( "QTabBar::tab { height: 25px }" );
        }
        setStyleSheet( "QTableView: { color: blue; font-size: 8pt}" );
    }
    // connect( impl_, &impl::onReply, this, &dgWidget::handleReply );
    // connect( impl_, &impl::onProtocols, this, &dgWidget::handleProtocols );
}

dgWidget::~dgWidget()
{
}

void
dgWidget::OnCreate( const adportable::Configuration& )
{
}

void
dgWidget::OnInitialUpdate()
{
    // if ( auto edit = findChild< QLineEdit * >( "url" ) )
    //     edit->setText( QString( "http://%1/dg/ctl?events" ).arg( impl_->server().c_str() ) );
}

void
dgWidget::OnFinalClose()
{
}

bool
dgWidget::getContents( boost::any& ) const
{
    return false;
}

bool
dgWidget::setContents( boost::any&& )
{
    return false;
}

QSize
dgWidget::sizeHint() const
{
    return QSize( this->size().width(), 80 );
}

void
dgWidget::handleReply( const QString& ev, const QString& data )
{
    // static size_t count = 0;
    
    // if ( auto edit = findChild< QLineEdit * >( "eventText" ) ) {
    //     edit->setText( data );

    //     if ( ( count++ % 3 ) == 0 )
    //         impl_->query_protocols();
    // }
}

void
dgWidget::handleProtocols( const QByteArray& json )
{
    // if ( auto tab = findChild< QTabWidget * >() ) {

    //     adio::dg::protocols< adio::dg::protocol<> > p;
    //     std::istringstream is( json.constData() );

    //     if ( p.read_json( is, p ) ) {

    //         while ( tab->count() > p.size() )
    //             tab->removeTab( tab->count() - 1 );

    //         while ( tab->count() < p.size() )
    //             tab->addTab( new ProtocolForm(), QString( "Proto %1" ).arg( QString::number( tab->count() + 1 ) ) );

    //         if ( auto w = findChild< QDoubleSpinBox * >("interval") )
    //             w->setValue( p.interval() * 1.0e3 );

    //         for ( size_t i = 0; i < p.size(); ++i ) {
    //             if ( auto w = qobject_cast< ProtocolForm *>( tab->widget( i ) ) ) {
    //                 w->setProtocol( p[ i ] );
    //             }
    //         }
    //     }
    // }
}

void
dgWidget::setURL( const QString& url )
{
    if ( auto edit = findChild< QLineEdit * >("url") )
        edit->setText( url );
}

void
dgWidget::setJsonObject( const QJsonObject& )
{
}

