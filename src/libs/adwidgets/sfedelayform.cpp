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

#include "sfedelayform.hpp"
#include "create_widget.hpp"
#include "utilities.hpp" // tuple<>(x,y)++
#include "spin_t.hpp"
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qpushbutton.h>
#include <adportable/debug.hpp>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpacerItem>
#include <boost/json.hpp>
#include <set>

namespace adwidgets {

    class SFEDelayForm::impl  {
    public:
        ~impl() {}
        impl() : value_ {
                { "exclude_regex", R"(^.*\(SPD-40_A.*$)" }
                , { "query_regex", R"(^SFE([0-9]+).*SFC)" }
                , { "value", 10.0 }
            } {
        }

        void set_exclude( const QString& regex, SFEDelayForm * form ) {
            value_[ "exclude_regex" ] = regex.toStdString();
            emit form->dataChanged( boost::json::serialize( value_ ).data() );
        }
        void set_query( const QString& regex, SFEDelayForm * form ) {
            value_[ "query_regex" ] = regex.toStdString();
            emit form->dataChanged( boost::json::serialize( value_ ).data() );
        }
        void set_value( double value, SFEDelayForm * form ) {
            value_[ "value" ] = value;
            emit form->dataChanged( boost::json::serialize( value_ ).data() );
        }

        const boost::json::object& jobj() const { return value_; }
        QString exclude_regex() { return QString::fromStdString( value_[ "exclude_regex" ].as_string().data() ); }
        QString query_regex() { return QString::fromStdString( value_[ "query_regex" ].as_string().data() ); }
        double value() { return value_[ "value" ].as_double(); }

    private:
        boost::json::object value_;
    };

}

using namespace adwidgets;

SFEDelayForm::~SFEDelayForm()
{
    delete impl_;
}



SFEDelayForm::SFEDelayForm( QWidget * parent ) : QFrame( parent )
                                               , impl_( new impl{} )
{
    if ( auto vLayout = new QVBoxLayout( QVBoxLayout( this ) ) ) {

        if ( auto gridLayout = add_layout( vLayout, create_widget< QGridLayout >("layout_1") ) ) {
            std::tuple< size_t, size_t > xy{0,0};

            add_widget( gridLayout, create_widget< QLabel >( "Exclude", "Exclude file if match:" ), std::get<0>(xy), std::get<1>(xy)++ );
            if ( auto edt
                 = add_widget( gridLayout, create_widget< QLineEdit >( "exclude", impl_->exclude_regex() )
                               , std::get<0>(xy), std::get<1>(xy)++) ) {
                connect( edt, &QLineEdit::textChanged, [&](const QString text ){ impl_->set_exclude( text, this ); });
            }

            ++xy;
            add_widget( gridLayout, create_widget< QLabel >( "Query", "Query:" ), std::get<0>(xy), std::get<1>(xy)++ );
            if ( auto w
                 = add_widget( gridLayout, create_widget< QLineEdit >( "query", impl_->query_regex() )
                               , std::get<0>(xy), std::get<1>(xy)++ ) )
                connect( w, &QLineEdit::textChanged, [&](const QString text ){ impl_->set_query( text, this ); });

            ++xy;
            add_widget( gridLayout, create_widget< QLabel >( "Delay", "Alt. delay (s):" ), std::get<0>(xy), std::get<1>(xy)++ );
            using namespace spin_initializer;
            if ( auto w
                 = add_widget( gridLayout, create_widget< QDoubleSpinBox >( "delay" ), std::get<0>(xy), std::get<1>(xy)++ ) ) {
                spin_init( w, std::make_tuple( Value<>{ impl_->value() }, Minimum<>{0.0}, Maximum<>{999.9}, Decimals{1}, Alignment{Qt::AlignRight}) );
                connect( w, &QDoubleSpinBox::valueChanged, [&](double value){ impl_->set_value( value, this ); } );
            }
        }
        if ( auto btn = add_widget( vLayout, create_widget< QDialogButtonBox >( "btnBox" ) ) ) {
            btn->addButton( new QPushButton( "Update", btn ), QDialogButtonBox::ApplyRole );
            connect( btn, &QDialogButtonBox::clicked, [this](){
                emit apply ( data() );
            });
        }
    }
}

QByteArray
SFEDelayForm::data() const
{
    auto json = boost::json::serialize( boost::json::value_from( impl_->jobj() ) );
    return QByteArray( json.data(), json.size() );
}
