/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "msreferencedialog.hpp"
#include "ui_msreferencedialog.h"
#include <adcontrols/constants.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/tableofelement.hpp>
#include <adcontrols/isotopes.hpp>
#include <adportable/utf.hpp>
#include <boost/variant.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>

namespace adwidgets {
    namespace detail {
        namespace msreferencedialog {

            enum idItem {
                idEndGroupLabel
                , idRepeatLabel
                , idAdductLabel
                , idPolarityLabel
                , idClearFormButton
                , idEndGroupLineEdit
                , idRepeatLineEdit
                , idAdductLineEdit
                , idAddReferenceButton
                , idAdductsMaterialsCombo
                , numItems
            };

            typedef boost::variant<QLabel *, QComboBox *, QPushButton *, QLineEdit * > control_variant;

            struct ui_accessor {
                Ui_MSReferenceDialog * ui_;
                control_variant ref_;
                ui_accessor( Ui_MSReferenceDialog * ui ) : ui_( ui ) {}
                control_variant& operator () ( idItem id ) {
                    switch( id ) {
                    case idEndGroupLabel: ref_ = ui_->label_12; return ref_;
                    case idRepeatLabel:   ref_ = ui_->label_11; return ref_;
                    case idAdductLabel:   ref_ = ui_->label; return ref_;
                    case idPolarityLabel:    ref_ = ui_->label_2; return ref_;
                    case idClearFormButton:  ref_ = ui_->pushButton_3; return ref_;
                    case idEndGroupLineEdit: ref_ = ui_->edtEndGroup_3; return ref_;
                    case idRepeatLineEdit:   ref_ = ui_->edtRepeatGroup_3; return ref_;
                    case idAdductLineEdit:   ref_ = ui_->edtAdductLose_3; return ref_;
                    case idAddReferenceButton:  ref_ = ui_->addReference_3; return ref_;
                    case idAdductsMaterialsCombo: ref_ = ui_->comboBox_3; return ref_;
                    case numItems:
                        break;
                    }
                    class error : public boost::exception, public std::exception {};
                    BOOST_THROW_EXCEPTION( error() );
                }
            };

            struct align_property : public boost::static_visitor < void > {
                QFlags<Qt::AlignmentFlag> align_;
                align_property() : align_( Qt::AlignRight | Qt::AlignVCenter ) {
                }
                template< class T > void operator ()( T* widget ) const {
                    widget->setAlignment( align_ );
                }
            };
            template<> void align_property::operator() ( QComboBox * ) const {}
            template<> void align_property::operator() ( QPushButton * ) const {}
            template<> void align_property::operator() ( QLineEdit * w ) const { w->setAlignment( Qt::AlignLeft | Qt::AlignVCenter ); }

            struct set_text :  public boost::static_visitor< void > {
                const QString& text_;
                set_text( const QString& text ) : text_( text ) {}
                template< class T > void operator() ( T* w ) const { w->setText( text_ ); }
            };
            template<> void set_text::operator() ( QComboBox * ) const {}

            struct get_text : public boost::static_visitor < std::string > {
                template< class T > std::string operator() ( T* w ) const { return w->text().toStdString(); }
            };
            template<> std::string get_text::operator() ( QComboBox * ) const { return std::string(); }


        } // namespace
    }
}

using namespace adwidgets;
using namespace adwidgets::detail::msreferencedialog;

MSReferenceDialog::MSReferenceDialog( QWidget *parent ) : QDialog( parent, Qt::Tool )
                                                        , ui( new Ui::MSReferenceDialog )
{
    ui->setupUi( this );
    // font_property()(ui->groupBox);

    ui_accessor accessor( ui );

    // for ( int i = 0; i < numItems; ++i ) {
    //     control_variant v = accessor( idItem(i) );
    //     boost::apply_visitor( font_property(false), accessor( idItem( i ) ) );
    //     boost::apply_visitor( align_property(), accessor( idItem( i ) ) );
    // }

    auto materials = boost::get< QComboBox * >( accessor( idAdductsMaterialsCombo ) );
    while ( materials->count() )
        materials->removeItem( 0 );
    materials->addItem( "PFTBA", "PFTBA" );
    materials->addItem( "Ar", "Ar" );
    materials->addItem( "Xe", "Xe" );
    materials->addItem( "PEG", "H2O\tC2H4O\tH" );
    materials->addItem( "Recerpine", "C33H40N2O9\t\tH" );
    materials->addItem( "Polystyrene", "H2O\tC8H8\tH" );
    materials->addItem( "Jeffamine(D230)", "CH3CH(NH2)CH2NH2\tOCH2CH(CH3)\tH" );
    materials->addItem( "Sulfa drug (311)", "C12H14N4O4S\t\tH" );
    materials->addItem( "Agilent TOF Mix(+)", "Agilent TOF Mix(+)" );
    materials->addItem( "Agilent TOF Mix(-)", "Agilent TOF Mix(-)" );
    materials->addItem( "Anionic Surfactants 1(-)", "C12H26SO4\tC2H4O\t-H\t" ); // negative
    materials->addItem( "Anionic Surfactants 2(-)", "C13H28SO4\tC2H4O\t-H\t" ); // negative only
    materials->addItem( "Sodium acetate", "\tCH3COONa\tNa\t" ); //
    materials->addItem( "TFANa(+)", "\tCF3COONa\t[Na]+\t" ); //
    materials->addItem( "TFANa(-)", "\tCF3COONa\t[CF3COO]-" ); //
    materials->addItem( "Acetonitrile", "(CH3CN)2\t\t[H]+\t" ); //
    materials->addItem( "YOKUDELUNA(+)", "\tC2F3O2Na\t[Na]+\t" );

    connect( materials, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MSReferenceDialog::handleIndexChanged );

    auto button = boost::get< QPushButton * >( accessor( idAddReferenceButton ) );
    connect( button, &QPushButton::pressed, this, &MSReferenceDialog::handleAddReference );
}

MSReferenceDialog::~MSReferenceDialog()
{
    delete ui;
}

void
MSReferenceDialog::setHMass( double value )
{
    hMass_ = value;
}

void
MSReferenceDialog::register_handler( std::function< void( const adcontrols::MSReference& ) > f )
{
    reference_receiver_ = f;
}

void
MSReferenceDialog::handleIndexChanged( int index )
{
    ui_accessor accessor( ui );

    boost::apply_visitor( set_text(""), accessor( idEndGroupLineEdit ) );
    boost::apply_visitor( set_text(""), accessor( idRepeatLineEdit ) );
    boost::apply_visitor( set_text(""), accessor( idAdductLineEdit ) );

    auto materials = boost::get<QComboBox *>( accessor( idAdductsMaterialsCombo ) );
    auto refText   = materials->currentText();

    std::wstring userData = materials->itemData( index ).toString().toStdWString();
    if ( userData.empty() )
        return;

    typedef boost::tokenizer< boost::char_separator<wchar_t>
                              , std::wstring::const_iterator
                              , std::wstring > tokenizer_t;

	boost::char_separator<wchar_t> separator( L"\t", L"", boost::keep_empty_tokens );
    tokenizer_t tokens( userData, separator );

    std::wstring endGroup, repeat, adducts;

    auto token = tokens.begin();
    if ( token != tokens.end() ) {
        endGroup = *token;
        boost::apply_visitor( set_text( QString::fromStdWString(*token) ), accessor( idEndGroupLineEdit ) );
    }

	if ( ++token != tokens.end() ) {
        repeat = *token;
        boost::apply_visitor( set_text( QString::fromStdWString(*token) ), accessor( idRepeatLineEdit ) );
    }

    if ( token != tokens.end() && ++token != tokens.end() ) {
        adducts = *token;
        boost::apply_visitor( set_text( QString::fromStdWString(*token) ), accessor( idAdductLineEdit ) );
    }
    auto positive = boost::get< QLabel *>( accessor( idPolarityLabel ) );
    if ( adducts[ 0 ] == L'-' || refText.contains( "(-)" ) ) {
        positive->setText( "Negative" );
    }
    else {
        positive->setText( "Positive" );
    }
}

void
MSReferenceDialog::handleAddReference()
{
    ui_accessor accessor( ui );

    std::string endGroup = boost::apply_visitor( get_text(), accessor( idEndGroupLineEdit ) );
    std::string repeat = boost::apply_visitor( get_text(), accessor( idRepeatLineEdit ) );
    std::string adduct = boost::apply_visitor( get_text(), accessor( idAdductLineEdit ) );

    QString refname;
    if ( auto combo = boost::get<QComboBox *>( accessor( idAdductsMaterialsCombo ) ) )
        refname = combo->currentText();

    if ( reference_receiver_ ) {

        using adcontrols::polarity_positive;
        using adcontrols::polarity_negative;

        if ( refname == "TFANa(-)" ) {
            reference_receiver_( adcontrols::MSReference( "[CF3]-",       polarity_negative, "", true, 0.0, 1, "CF3" ) );
            reference_receiver_( adcontrols::MSReference( "[CF3COO]-",    polarity_negative, "", true, 0.0, 1, "C2F3O2" ) );
        }

        if ( !repeat.empty() ) {
            int lMass = 1;
            int hMass = hMass_;
            int nRepeat = 1;
            adcontrols::MSReference ref;
            do {
                std::string formula = ( boost::format( "%1%(%2%)%3%" ) % endGroup % repeat % nRepeat++  ).str();
                ref = adcontrols::MSReference( formula.c_str(), polarity_positive, adduct.c_str(), true );
                if ( lMass <= ref.exact_mass() && ref.exact_mass() < hMass )
                    reference_receiver_( ref );
            } while ( ref.exact_mass() < hMass );

        } else {
            if ( endGroup == "PFTBA" ) {
                reference_receiver_( adcontrols::MSReference( "CF3",     polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C2F4",    polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C2F5",    polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C3F5",    polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C4F9",    polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C5F10N",  polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C6F12N",  polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C7F12N",  polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C8F14N",  polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C8F16N",  polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C9F16N",  polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C9F18N",  polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C9F20N",  polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C12F22N", polarity_positive,  "", false ) );
                reference_receiver_( adcontrols::MSReference( "C12F24N", polarity_positive,  "", false ) );
            } else if ( endGroup == "Agilent TOF Mix(-)" ) {
                reference_receiver_( adcontrols::MSReference( "C6F9N3",          polarity_negative, "OH", false ) );
                reference_receiver_( adcontrols::MSReference( "C12F21N3",        polarity_negative, "OH", false ) );
                reference_receiver_( adcontrols::MSReference( "C2F3O2NH4",       polarity_negative, "-NH4", false ) );
                reference_receiver_( adcontrols::MSReference( "C12H18F12N3O6P3", polarity_negative, "C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( "C18H18F24N3O6P3", polarity_negative, "C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( "C24H18F36N3O6P3", polarity_negative, "C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( "C30H18F48N3O6P3", polarity_negative, "C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( "C36H18F60N3O6P3", polarity_negative, "C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( "C42H18F72N3O6P3", polarity_negative, "C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( "C48H18F84N3O6P3", polarity_negative, "C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( "C54H18F96N3O6P3", polarity_negative, "C2F3O2", false ) );
            } else if ( endGroup == "Agilent TOF Mix(+)" ) {
                reference_receiver_( adcontrols::MSReference( "C5H11NO2",        polarity_positive, "H", false, 0.0, 1, "118.0868" ) );
                reference_receiver_( adcontrols::MSReference( "C6H18N3O6P3",     polarity_positive, "H", false, 0.0, 1, "322.0486" ) );
                reference_receiver_( adcontrols::MSReference( "C12H18F12N3O6P3", polarity_positive, "H", false, 0.0, 1, "622.0295" ) );
                reference_receiver_( adcontrols::MSReference( "C18H18F24N3O6P3", polarity_positive, "H", false, 0.0, 1, "922.0103" ) );
                reference_receiver_( adcontrols::MSReference( "C24H18F36N3O6P3", polarity_positive, "H", false ) );
                reference_receiver_( adcontrols::MSReference( "C30H18F48N3O6P3", polarity_positive, "H", false ) );
                reference_receiver_( adcontrols::MSReference( "C36H18F60N3O6P3", polarity_positive, "H", false ) );
                reference_receiver_( adcontrols::MSReference( "C42H18F72N3O6P3", polarity_positive, "H", false ) );
                reference_receiver_( adcontrols::MSReference( "C48H18F84N3O6P3", polarity_positive, "H", false ) );
                reference_receiver_( adcontrols::MSReference( "C54H18F96N3O6P3", polarity_positive, "H", false ) );

            } else {
                // check if an element
                if ( adcontrols::mol::element element = adcontrols::TableOfElement::instance()->findElement( endGroup ) ) {
                    for ( auto& i: element.isotopes() ) {
                        std::string formula = ( boost::format("%1%%2%") % int( i.mass + 0.3 ) % element.symbol() ).str();
                        std::string description = ( boost::format("%.4f") % i.abundance ).str();
                        bool enable = i.abundance > 0.01;
                        reference_receiver_( adcontrols::MSReference( formula, polarity_positive, "", enable, i.mass, 1, description ) );
                    }
                } else {
                    // chemical formula
                    reference_receiver_( adcontrols::MSReference( endGroup, polarity_positive, adduct ) );
                }
            }
        }
    }
}
