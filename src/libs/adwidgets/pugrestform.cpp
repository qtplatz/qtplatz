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

#include "pugrestform.hpp"
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

    class PUGRestForm::impl : public QObject {
        Q_OBJECT
    public:
        ~impl() {}
        impl() {}
        void autocomplete( bool checked ) {
            d_.set_pug_autocomplete( checked );
            emit dataChanged();
        }
        void set_identify( const QString& text ) {
            d_.set_pug_identifier( text.toStdString() );
            emit dataChanged();
        }
        void set_autocomplete( bool checked ) {
            d_.set_pug_autocomplete( checked );
            emit dataChanged();
        }

        void set_property( const QString& property, bool checked ) {
            d_.set_pug_property( property.toStdString(), checked );
            emit dataChanged();
        }
        void set_domain( const QString& domain, bool checked ) {
            if ( checked )
                d_.set_pug_domain( domain.toStdString() );
            emit dataChanged();
        }
        void set_namespace( const QString& ns, bool checked ) {
            if ( checked )
                d_.set_pug_namespace( ns.toStdString() );
            emit dataChanged();
        }

        void set_url( const QString& url ) {
            d_.set_pug_url( url.toStdString() );
        }

    signals:
        void dataChanged();

    public:
        adcontrols::PUGREST d_;
    };

}

using namespace adwidgets;

PUGRestForm::~PUGRestForm()
{
    delete impl_;
}

PUGRestForm::PUGRestForm( QWidget * parent ) : QFrame( parent )
                                             , impl_( new impl{} )
{
    if ( auto vLayout = new QVBoxLayout( QVBoxLayout( this ) ) ) {

        if ( auto gridLayout = add_layout( vLayout, create_widget< QGridLayout >("layout_1") ) ) {
            std::tuple< size_t, size_t > xy{0,0};
            add_widget( gridLayout, create_widget< QLabel >( "URL", "URL" ), std::get<0>(xy), std::get<1>(xy)++ );
            if ( auto edt
                 = add_widget( gridLayout, create_widget< QLineEdit >( "url", "pubchem.ncbi.nlm.nih.gov" )
                               , std::get<0>(xy), std::get<1>(xy)++) ) {
                connect( edt, &QLineEdit::textEdited, impl_, &impl::set_url );
            }

            ++xy;
            std::get<1>(xy)++;
            if ( auto cbx
                 = add_widget( gridLayout, create_widget< QCheckBox >( "autocomplete", "autocomplete" ), std::get<0>(xy), std::get<1>(xy)++) ) {
                connect( cbx, &QCheckBox::toggled, [&](bool checked){ impl_->set_autocomplete( checked ); });
            }

            ++xy;
            add_widget( gridLayout, create_widget< QLabel >( "query", "Query:" ), std::get<0>(xy), std::get<1>(xy)++ );
            if ( auto w
                 = add_widget( gridLayout, create_widget< QLineEdit >( "identifier", "apap" ), std::get<0>(xy), std::get<1>(xy)++ ) )
                connect( w, &QLineEdit::textChanged, [&](const QString text ){ impl_->set_identify( text ); });
        }

        if ( auto hLayout = add_layout( vLayout, create_widget< QHBoxLayout >("layout_2") ) ) {

            if ( auto gbx = add_widget( hLayout, create_widget< QGroupBox >("property", "property" ) ) )  {

                if ( auto gLayout = new QGridLayout{ gbx } ) {
                    gLayout->setContentsMargins( {} );

                    std::tuple< size_t, size_t > lxy{0,0};
                    for ( auto prop: { "CanonicalSMILES", "MolecularFormula", "MolecularWeight"
                                       , "InChI", "InChIKey", "IUPACName"
                                       , "Title", "XLogP", "ExactMass" } ) {
                        if ( auto cbx
                             = add_widget( gLayout, create_widget< QCheckBox >( prop, prop ), std::get<0>(lxy), std::get<1>(lxy)++ ) )
                            connect( cbx, &QCheckBox::toggled, [cbx,this]( bool checked ){
                                impl_->set_property( cbx->objectName(), checked ); });
                        if ( std::get<1>(lxy) >= 3 )
                            ++lxy;
                    }
                }
            }

            if ( auto gbx = add_widget( hLayout, create_widget< QGroupBox >("domain", "domain" ) ) )  {
                if ( auto gLayout = new QGridLayout{ gbx } ) {
                    gLayout->setContentsMargins( {} );
                    std::tuple< size_t, size_t > lxy{0,0};
                    for( auto domain: { "compound", "substance", "protein", "taxonomy" } ) {
                        if ( auto rbtn = add_widget( gLayout, create_widget< QRadioButton >(domain, domain, this )
                                                     , std::get<0>(lxy), std::get<1>(lxy)++ ) ) {
                            connect( rbtn, &QRadioButton::toggled, this, [rbtn,this]( bool checked ){
                                impl_->set_domain( rbtn->objectName(), checked );    });
                        }
                        if ( std::get<1>(lxy) >= 2 )
                            ++lxy;
                    }
                }
            }

            // ++xy;
            if ( auto gbx = add_widget( hLayout, create_widget< QGroupBox >("namespace", "namespace" ) ) ) {
                if ( auto gLayout = new QGridLayout{ gbx } ) {
                    gLayout->setContentsMargins( {} );
                    std::tuple< size_t, size_t > lxy{0,0};
                    for( auto ns: { "name", "cid", "smiles", "inchi", "inchikey", "formula" } ) {
                        if ( auto rbtn = add_widget( gLayout, create_widget< QRadioButton >( ns, ns, this )
                                                     , std::get<0>(lxy), std::get<1>(lxy)++ ) ) {
                            connect( rbtn, &QRadioButton::toggled, this, [rbtn, this]( bool checked ){
                                impl_->set_namespace( rbtn->objectName(), checked );   });
                        }
                        if ( std::get<1>(lxy) >= 3 )
                            ++lxy;
                    }
                }
            }
        }

        if ( auto btn = add_widget( vLayout, create_widget< QDialogButtonBox >( "btnBox" ) ) ) {
            btn->setStandardButtons( QDialogButtonBox::Apply );
            connect( btn, &QDialogButtonBox::clicked, [this](){
                auto json = boost::json::serialize( boost::json::value_from( impl_->d_ ) );
                emit apply ( QByteArray( json.data(), json.size() ) );
            });
        }
    }

    setData( adcontrols::PUGREST{} );

    connect( impl_, &impl::dataChanged, this, [&](){
        if ( auto url = accessor{this}.find< QLineEdit * >( "url" ) ) {
            url->setText( QString::fromStdString( adcontrols::PUGREST::to_url( impl_->d_, true ) ) );
            impl_->set_url( url->text() );
        }
    });
}

adcontrols::PUGREST
PUGRestForm::data() const
{
    return impl_->d_;
}

void
PUGRestForm::setData( const adcontrols::PUGREST& t )
{
    impl_->d_ = t;
    if ( auto url = accessor{this}.find< QLineEdit * >( "url" ) ) {
        url->setText( QString::fromStdString( adcontrols::PUGREST::to_url( t, true ) ) );
    }
    if ( auto gbx = accessor{this}.find< QGroupBox * >( "identifier" ) ) {
        gbx->setChecked( t.pug_autocomplete() );
    }
    if ( auto query = accessor{this}.find< QLineEdit * >( "query" ) ) {
        query->setText( QString::fromStdString( t.pug_identifier() ) );
    }
    if ( auto gbx = accessor{this}.find< QGroupBox * >( "property" ) ) {
        QSignalBlocker block( gbx );
        for ( const auto& cbx: gbx->findChildren< QCheckBox * >() ) {
            bool checked = std::find( t.pug_properties().begin()
                                      , t.pug_properties().end()
                                      , cbx->objectName().toStdString() ) != t.pug_properties().end();
            cbx->setChecked( checked );
        }
    }
    if ( auto gbx = accessor{this}.find< QGroupBox * >( "domain" ) ) {
        if ( auto radio = accessor{ gbx }.find< QRadioButton * >( QString::fromStdString( t.pug_domain() ) ) ) {
            radio->setChecked( true );
        }
    }

    if ( auto gbx = accessor{this}.find< QGroupBox * >( "namespace" ) ) {
        if ( auto radio = accessor{ gbx }.find< QRadioButton * >( QString::fromStdString( t.pug_namespace() ) ) ) {
            radio->setChecked( true );
        }
    }
}

#include "pugrestform.moc"
