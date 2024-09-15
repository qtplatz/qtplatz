/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "sfedelaydialog.hpp"
#include "sfedelayform.hpp"
#include "tableview.hpp"
#include <QtCore/qnamespace.h>
#include <adportable/debug.hpp>
#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QLabel>
#include <boost/uuid/uuid.hpp>
#include <stdexcept>
#include <sstream>
#include <filesystem>
#include <regex>

namespace {

    struct json_value {
        boost::json::object& jobj_;
        json_value( boost::json::object& t ) : jobj_( t ) {}
        void set_object( boost::json::value& jv ) { jobj_ = jv.as_object(); }
        void set_object( const std::string& json ) { jobj_ = boost::json::parse( json.data() ).as_object(); }
        std::string exclude_regex() const { return jobj_[ "exclude_regex" ].as_string().data(); };
        std::string query_regex() const { return jobj_[ "query_regex" ].as_string().data(); };
        double value() const { return jobj_[ "value" ].as_double(); }
    };

    struct match_exclude {
        std::regex re_;
        match_exclude( std::regex&& re ) : re_( std::move( re ) ) {}
        bool operator ()( const std::string& name ) const {
            std::match_results< typename std::basic_string< char >::const_iterator > match;
            return std::regex_match( name, match, re_ );
        }
    };

    struct query_value {
        std::regex re_;
        query_value( std::regex&& re ) : re_( std::move( re ) ) {}
        std::optional< double > operator ()( const std::string& name ) const {
            std::match_results< typename std::basic_string< char >::const_iterator > match;
            if ( std::regex_search( name, match, re_ ) ) {
                if ( match.size() >= 2 )
                    return std::stod( match[1].str() );
            }
            return {};
        }
    };

}


using namespace adwidgets;

SFEDelayDialog::SFEDelayDialog( QWidget *parent ) : QDialog( parent )
{
    resize( 1024, size().height() );
    if ( auto layout = new QVBoxLayout( this ) ) {

        layout->setContentsMargins( {} );
        layout->setSpacing( 2 );

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new SFEDelayForm ) );
            splitter->addWidget( ( new TableView ) );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 4 );
            splitter->setOrientation ( Qt::Vertical );
            layout->addWidget( splitter );
        }

        auto buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
        layout->addWidget( buttons );

        connect( buttons, &QDialogButtonBox::accepted, this, [&] () { QDialog::accept(); } );
        connect( buttons, &QDialogButtonBox::rejected, this, [&] () { QDialog::reject(); } );
    }
    if ( auto form = findChild< SFEDelayForm * >() ) {
        connect( form, &SFEDelayForm::dataChanged, this, &SFEDelayDialog::form_changed );
        connect( form, &SFEDelayForm::apply, this, &SFEDelayDialog::form_changed );
        json_value( jobj_ ).set_object( form->data().toStdString() );

        ADDEBUG() << jobj_;
    }
}

void
SFEDelayDialog::setContents( const std::vector< std::string >& files )
{
    if ( auto tv = findChild< TableView * >() ) {
        auto model = new QStandardItemModel();
        model->setColumnCount( 2 );
        model->setHeaderData( 0, Qt::Horizontal, QObject::tr( "delay (s)" ) );
        model->setHeaderData( 1, Qt::Horizontal, QObject::tr( "name" ) );

        model->setRowCount( files.size() );
        for ( size_t row = 0; row < files.size(); ++ row ) {
            model->setData( model->index( row, 0 ), 10.0 );
            if ( auto item = model->item( row, 0 ) ) {
                if ( !( item->flags() & Qt::ItemIsUserCheckable ) ) {
                    item->setEditable( true );
                    item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                }
            }
            model->setData( model->index( row, 1 )
                            , QString::fromStdString( std::filesystem::path( files.at( row ) ).filename().stem() ) );
            model->setData( model->index( row, 1 )
                            , QByteArray( files.at( row ).data(), files.at( row ).size() ), Qt::UserRole );
        }

        tv->setSortingEnabled( false );
        tv->setModel( model );
        tv->resizeColumnsToContents();
    }
    update_table();
}

std::vector< std::tuple< std::string, bool, double > >
SFEDelayDialog::getContents() const
{
    if ( auto tv = findChild< TableView * >() ) {
        std::vector< std::tuple< std::string, bool, double > > list;
        auto model = tv->model();
        for ( size_t row = 0; row < model->rowCount(); ++row ) {
            list.emplace_back( model->data( model->index( row, 1 ), Qt::UserRole ).toByteArray().toStdString()
                               , model->data( model->index( row, 0 ), Qt::CheckStateRole ) == Qt::Checked
                               , model->data( model->index( row, 0 ) ).toDouble() );
        }
        return list;
    }
    return {};
}

void
SFEDelayDialog::form_changed( const QByteArray& json )
{
    json_value( jobj_ ).set_object( json.toStdString() );
    update_table();
}

void
SFEDelayDialog::update_table()
{
    match_exclude exclude( std::regex( json_value( jobj_ ).exclude_regex(), std::regex::extended ) );
    query_value qv( std::regex( json_value( jobj_ ).query_regex(), std::regex::extended ) );
    double alt_value = json_value( jobj_ ).value();

    if ( auto tv = findChild< TableView * >() ) {
        auto model = tv->model();
        for ( size_t row = 0; row < model->rowCount(); ++row ) {
            auto name = model->data( model->index( row, 1 ) ).toString().toStdString();
            model->setData( model->index( row, 0 ), (exclude(name) ? Qt::Unchecked : Qt::Checked), Qt::CheckStateRole );

            if ( exclude( name ) ) {
                model->setData( model->index( row, 0 ), 0.0 );
            } else {
                if ( auto value = qv( name ) ) {
                    model->setData( model->index( row, 0 ), *value );
                } else {
                    model->setData( model->index( row, 0 ), alt_value );
                }
            }
        }
    }
}
