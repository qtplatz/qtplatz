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

namespace adwidgets {

    class PUGRestForm::impl : public QObject {
        Q_OBJECT
    public:
        ~impl() {}
        impl() {}
        void autocomplete( bool checked ) {
            d_.set_pug_autocomplete( checked );
        }
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
            gridLayout->addWidget( create_widget< QLabel >( "URL", "URL" ), std::get<0>(xy), std::get<1>(xy)++);
            add_widget( gridLayout, create_widget< QLineEdit >( "url", "pubchem.ncbi.nlm.nih.gov" ), std::get<0>(xy), std::get<1>(xy)++);
            ++xy;
            std::get<1>(xy)++;
            add_widget( gridLayout, create_widget< QCheckBox >( "autocomplete", "autocomplete" ), std::get<0>(xy), std::get<1>(xy)++);
        }

        if ( auto gbx = add_widget( vLayout, create_widget< QGroupBox >("identifier", "identifier" ) ) )  {
            if ( auto gLayout = new QGridLayout{ gbx } ) {
                add_widget( gLayout, create_widget< QLabel >( "query", "Query:" ), 0, 0 );
                if ( auto w = add_widget( gLayout, create_widget< QLineEdit >( "identifier", "apap" ), 0, 1 ) )
                    connect( w, &QLineEdit::textChanged, [&](const QString text ){ });
            }
            connect( gbx, &QGroupBox::toggled, [&](bool checked){ impl_->d_.set_pug_autocomplete( checked ); } );
        }

        if ( auto hLayout = add_layout( vLayout, create_widget< QHBoxLayout >("2ndLayout") ) ) {

            if ( auto gbx = add_widget( hLayout, create_widget< QGroupBox >("property", "property" ) ) )  {
                if ( auto gLayout = new QGridLayout{ gbx } ) {
                    gLayout->setContentsMargins( {} );
                    std::tuple< size_t, size_t > lxy{0,0};
                    add_widget( gLayout, create_widget< QCheckBox >( "CanonicalSMILES", "CanonicalSMILES" ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QCheckBox >( "MolecularFormula", "MolecularFormula" ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QCheckBox >( "MolecularWeight", "MolecularWeight" ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    ++lxy;
                    add_widget( gLayout, create_widget< QCheckBox >( "InChI", "InChI" ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QCheckBox >( "InChIKey", "InChIKey" ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QCheckBox >( "IUPACName", "IUPACName" ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    ++lxy;
                    add_widget( gLayout, create_widget< QCheckBox >( "Title", "Title" ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QCheckBox >( "XLogP", "XLogP" ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QCheckBox >( "ExactMass", "ExactMass" ), std::get<0>(lxy), std::get<1>(lxy)++ );
                }
            }

            if ( auto gbx = add_widget( hLayout, create_widget< QGroupBox >("domain", "domain" ) ) )  {
                if ( auto gLayout = new QGridLayout{ gbx } ) {
                    gLayout->setContentsMargins( {} );
                    std::tuple< size_t, size_t > lxy{0,0};
                    add_widget( gLayout, create_widget< QRadioButton >("compound", "compound", this ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QRadioButton >("substance", "substance", this ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    ++lxy;
                    add_widget( gLayout, create_widget< QRadioButton >("protein", "protein", this ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QRadioButton >("taxonomy", "taxonomy", this ), std::get<0>(lxy), std::get<1>(lxy)++ );
                }
            }
            // ++xy;
            if ( auto gbx = add_widget( hLayout, create_widget< QGroupBox >("namespace", "namespace" ) ) ) {
                if ( auto gLayout = new QGridLayout{ gbx } ) {
                    gLayout->setContentsMargins( {} );
                    std::tuple< size_t, size_t > lxy{0,0};
                    add_widget( gLayout, create_widget< QRadioButton >("name", "name", this ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QRadioButton >("cid", "cid", this ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QRadioButton >("smiles", "smiles", this ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    ++lxy;
                    add_widget( gLayout, create_widget< QRadioButton >("inchi", "inchi", this ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QRadioButton >("inchikey", "inchikey", this ), std::get<0>(lxy), std::get<1>(lxy)++ );
                    add_widget( gLayout, create_widget< QRadioButton >("formula", "formula", this ), std::get<0>(lxy), std::get<1>(lxy)++ );
                }
            }
        }
        if ( auto btn = add_widget( vLayout, create_widget< QDialogButtonBox >( "btnBox" ) ) ) {
            btn->setStandardButtons( QDialogButtonBox::Apply );
        }
    }
    setData( adcontrols::PUGREST{} );
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
