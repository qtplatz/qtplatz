/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC
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

#include "jstrestform.hpp"
#include "create_widget.hpp"
#include "utilities.hpp" // tuple<>(x,y)++
#include <QtWidgets/qradiobutton.h>
#include <adcontrols/pugrest.hpp>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qsizepolicy.h>
#include <adportable/debug.hpp>
#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QBoxLayout>
#include <QRadioButton>
#include <QSpacerItem>
#include <QSignalBlocker>
#include <boost/json.hpp>
#include <set>

namespace adwidgets {

    class JSTRestForm::impl : public QObject {
        Q_OBJECT
    public:
        ~impl() {}
        impl() {}
        void set_target( const QString& target ) {
            d_.set_target( target.toStdString() );
            emit dataChanged();
        }
        void set_port( const QString& port ) {
            d_.set_port( port.toStdString() );
            emit dataChanged();
        }
        void set_host( const QString& host ) {
            d_.set_host( host.toStdString() );
            emit dataChanged();
        }

        void set_url( const QString& url ) {
            d_.set_url( url.toStdString() );
            emit urlChanged();
        }

    signals:
        void dataChanged();
        void urlChanged();

    public:
        adcontrols::JSTREST d_;
    };

}

using namespace adwidgets;

JSTRestForm::~JSTRestForm()
{
    delete impl_;
}

JSTRestForm::JSTRestForm( QWidget * parent ) : QFrame( parent )
                                             , impl_( new impl{} )
{
    using adcontrols::JSTREST;

    if ( auto vLayout = new QVBoxLayout( QVBoxLayout( this ) ) ) {

        if ( auto gridLayout = add_layout( vLayout, create_widget< QGridLayout >("layout_1") ) ) {
            std::tuple< size_t, size_t > xy{0,0};
            add_widget( gridLayout, create_widget< QLabel >( "port", "PORT:" ), std::get<0>(xy), std::get<1>(xy)++ );
            if ( auto w
                 = add_widget( gridLayout, create_widget< QLineEdit >( "port", "https" ), std::get<0>(xy), std::get<1>(xy)++ ) )
                connect( w, &QLineEdit::textChanged, [&](const QString text ){ impl_->set_port( text ); });

            add_widget( gridLayout, create_widget< QLabel >( "host", "HOST:" ), std::get<0>(xy), std::get<1>(xy)++ );
            if ( auto w
                 = add_widget( gridLayout, create_widget< QLineEdit >( "host", "www.example.com" ), std::get<0>(xy), std::get<1>(xy)++ ) )
                connect( w, &QLineEdit::textChanged, [&](const QString text ){ impl_->set_host( text ); });
        }

        if ( auto gridLayout = add_layout( vLayout, create_widget< QGridLayout >("layout_2") ) ) {
            std::tuple< size_t, size_t > xy{0,0};
            add_widget( gridLayout, create_widget< QLabel >( "target", "QUERY:" ), std::get<0>(xy), std::get<1>(xy)++ );
            if ( auto w
                 = add_widget( gridLayout, create_widget< QLineEdit >( "target", "target" ), std::get<0>(xy), std::get<1>(xy)++ ) )
                connect( w, &QLineEdit::textChanged, [&](const QString text ){ impl_->set_target( text ); });

            ++xy;
            add_widget( gridLayout, create_widget< QLabel >( "URL", "URL" ), std::get<0>(xy), std::get<1>(xy)++ );
            if ( auto edt
                 = add_widget( gridLayout, create_widget< QLineEdit >( "url", JSTREST::to_url( impl_->d_ ).c_str() )
                               , std::get<0>(xy), std::get<1>(xy)++ ) )
                connect( edt, &QLineEdit::textChanged, impl_, &impl::set_url );
            ++xy;
        }

        ///////////////// button //////////////////
        if ( auto btn = add_widget( vLayout, create_widget< QDialogButtonBox >( "btnBox" ) ) ) {
            btn->setStandardButtons( QDialogButtonBox::Apply );
            connect( btn, &QDialogButtonBox::clicked, [this](){
                auto json = boost::json::serialize( boost::json::value_from( impl_->d_ ) );
                emit apply ( QByteArray( json.data(), json.size() ) );
            });
        }
    }

    setData( adcontrols::JSTREST{} );

    connect( impl_, &impl::dataChanged, this, [&](){
        if ( auto url = accessor{this}.find< QLineEdit * >( "url" ) ) {
            url->setText( QString::fromStdString( adcontrols::JSTREST::to_url( impl_->d_ ) ) );
        }
    });

    connect( impl_, &impl::urlChanged, this, [&](){
        if ( auto host = accessor{this}.find< QLineEdit * >( "host" ) ) {
            QSignalBlocker block( host );
            host->setText( QString::fromStdString( impl_->d_.host().c_str() ) );
        }
        if ( auto port = accessor{this}.find< QLineEdit * >( "port" ) ) {
            QSignalBlocker block( port );
            port->setText( QString::fromStdString( impl_->d_.port().c_str() ) );
        }
        if ( auto target = accessor{this}.find< QLineEdit * >( "target" ) ) {
            QSignalBlocker block( target );
            target->setText( QString::fromStdString( impl_->d_.target().c_str() ) );
        }
    });
}

adcontrols::JSTREST
JSTRestForm::data() const
{
    return impl_->d_;
}

void
JSTRestForm::setData( const adcontrols::JSTREST& t )
{
    QSignalBlocker block( this );
    impl_->d_ = t;
    if ( auto url = accessor{this}.find< QLineEdit * >( "url" ) ) {
        url->setText( QString::fromStdString( adcontrols::JSTREST::to_url( t ) ) );
    }
    if ( auto host = accessor{this}.find< QLineEdit * >( "host" ) ) {
        host->setText( QString::fromStdString( t.host() ) );
    }
    if ( auto port = accessor{this}.find< QLineEdit * >( "port" ) ) {
        port->setText( QString::fromStdString( t.port() ) );
    }
    if ( auto target = accessor{this}.find< QLineEdit * >( "target" ) ) {
        target->setText( QString::fromStdString( t.target() ) );
    }
}

#include "jstrestform.moc"
