/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
                , idAdductCombo
                , idPositiveCombo
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
                ui_accessor( Ui_MSReferenceDialog * ui ) : ui_( ui ) {}
                control_variant operator () ( idItem id ) {
                    switch( id ) {
                    case idEndGroupLabel: return ui_->label_12;
                    case idRepeatLabel:   return ui_->label_11;
                    case idAdductCombo:   return ui_->comboBox;
                    case idPositiveCombo:    return ui_->comboBox_2;
                    case idClearFormButton:  return ui_->pushButton_3;
                    case idEndGroupLineEdit: return ui_->edtEndGroup_3;
                    case idRepeatLineEdit:   return ui_->edtRepeatGroup_3;
                    case idAdductLineEdit:   return ui_->edtAdductLose_3;
                    case idAddReferenceButton:  return ui_->addReference_3;
                    case idAdductsMaterialsCombo: return ui_->comboBox_3;
                    }
                    return static_cast<QLabel *>(0);
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

            struct font_property : public boost::static_visitor < void > {
                QFont font;
                bool bold_;
                font_property( bool bold = true ) : bold_(bold) {
                    font.setFamily( "Calibri" );
                    font.setBold( bold_ );
                    font.setWeight( 75 );
                }
                template< class T > void operator ()( T* widget ) const {
                    widget->setFont( font );
                }
            };

            struct set_text :  public boost::static_visitor< void > {
                const QString& text_;
                set_text( const QString& text ) : text_( text ) {}
                template< class T > void operator() ( T* w ) const { w->setText( text_ ); }
            };
            template<> void set_text::operator() ( QComboBox * ) const {}

            struct get_text : public boost::static_visitor < std::wstring > {
                template< class T > std::wstring operator() ( T* w ) const { return w->text().toStdWString(); }
            };
            template<> std::wstring get_text::operator() ( QComboBox * ) const { return std::wstring(); }

            
        } // namespace
    }
}

using namespace adwidgets;
using namespace adwidgets::detail::msreferencedialog;

MSReferenceDialog::MSReferenceDialog(QWidget *parent) : QDialog(parent)
                                                      , ui(new Ui::MSReferenceDialog)
{
    ui->setupUi(this);
    font_property()(ui->groupBox);

    ui_accessor accessor( ui );
    for ( int i = 0; i < numItems; ++i ) {
        boost::apply_visitor( font_property( false ), accessor( idItem( i ) ) );
        boost::apply_visitor( align_property(), accessor( idItem( i ) ) );
    }

    auto materials = boost::get< QComboBox * >(accessor( idAdductsMaterialsCombo ));
    materials->addItem( "Ar", "Ar" );
	materials->addItem( "Xe", "Xe" );
	materials->addItem( "PFTBA", "PFTBA" );
	materials->addItem( "PEG", "H2O\tC2H4O\tH" );
	materials->addItem( "Recerpine", "C33H40N2O9\t\tH" );
	materials->addItem( "Polystyrene", "H2O\tC8H8\tH" );
	materials->addItem( "Jeffamine(D230)", "CH3CH(NH2)CH2NH2\tOCH2CH(CH3)\tH" );
	materials->addItem( "Sulfa drug (311)", "C12H14N4O4S\t\tH" );
	materials->addItem( "AgilentTOF Mix(+)", "AgilentTOF Mix(+)" );
	materials->addItem( "AgilentTOF Mix(-)", "AgilentTOF Mix(-)" );
	materials->addItem( "Anionic Surfactants 1(-)", "C12H26SO4\tC2H4O\t-H\t" ); // negative
	materials->addItem( "Anionic Surfactants 2(-)", "C13H28SO4\tC2H4O\t-H\t" ); // negative only
	materials->addItem( "Sodium acetate", "\tCH3COONa\tNa\t" ); //
   
    connect( materials, static_cast< void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MSReferenceDialog::handleIndexChanged );

    auto button = boost::get< QPushButton * >( accessor( idAddReferenceButton ) );
    connect( button, &QPushButton::pressed, this, &MSReferenceDialog::handleAddReference );
}

MSReferenceDialog::~MSReferenceDialog()
{
    delete ui;
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

    std::wstring userData = materials->itemData( index ).toString().toStdWString();
    if ( userData.empty() )
        return;

    typedef boost::tokenizer< boost::char_separator<wchar_t>
                              , std::wstring::const_iterator
                              , std::wstring > tokenizer_t;
 
	boost::char_separator<wchar_t> separator( L"\t", L"", boost::keep_empty_tokens );
    tokenizer_t tokens( userData, separator );
    
    auto token = tokens.begin();
    if ( token != tokens.end() )
        boost::apply_visitor( set_text( QString::fromStdWString(*token) ), accessor( idEndGroupLineEdit ) );
    
	if ( ++token != tokens.end() )
        boost::apply_visitor( set_text( QString::fromStdWString(*token) ), accessor( idRepeatLineEdit ) );

    if ( token != tokens.end() && ++token != tokens.end() )
        boost::apply_visitor( set_text( QString::fromStdWString(*token) ), accessor( idAdductLineEdit ) );
}

void
MSReferenceDialog::handleAddReference()
{
    ui_accessor accessor( ui );

    std::wstring endGroup = boost::apply_visitor( get_text(), accessor( idEndGroupLineEdit ) );
    std::wstring repeat = boost::apply_visitor( get_text(), accessor( idRepeatLineEdit ) );
    std::wstring adduct = boost::apply_visitor( get_text(), accessor( idAdductLineEdit ) );

    if ( reference_receiver_ ) {
        if ( !repeat.empty() ) {
            int lMass = 1;
            int hMass = 1000;
            int nRepeat = 1;
            adcontrols::MSReference ref;
            do {
                std::wstring formula = ( boost::wformat( L"%1%(%2%)%3%" ) % endGroup % repeat % nRepeat++  ).str();
                ref = adcontrols::MSReference( formula, true /* is positive */, adduct, true );
                if ( lMass <= ref.exact_mass() && ref.exact_mass() < hMass )
                    reference_receiver_( ref );
            } while ( ref.exact_mass() < hMass );

        } else {
            if ( endGroup == L"PFTBA" ) {
                reference_receiver_( adcontrols::MSReference( L"CF3",     true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C2F4",    true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C2F5",    true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C3F5",    true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C4F9",    true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C5F10N",  true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C6F12N",  true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C7F12N",  true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C8F14N",  true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C8F16N",  true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C9F16N",  true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C9F18N",  true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C9F20N",  true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C12F22N", true,  L"", false ) );
                reference_receiver_( adcontrols::MSReference( L"C12F24N", true,  L"", false ) );
            } else if ( endGroup == L"AgilentTOF Mix(-)" ) {
                reference_receiver_( adcontrols::MSReference( L"C6F9N3",          false, L"OH", false ) );
                reference_receiver_( adcontrols::MSReference( L"C12F21N3",        false, L"OH", false ) );
                reference_receiver_( adcontrols::MSReference( L"C2F3O2NH4",       false, L"-NH4", false ) );
                reference_receiver_( adcontrols::MSReference( L"C12H18F12N3O6P3", false, L"C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( L"C18H18F24N3O6P3", false, L"C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( L"C24H18F36N3O6P3", false, L"C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( L"C30H18F48N3O6P3", false, L"C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( L"C36H18F60N3O6P3", false, L"C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( L"C42H18F72N3O6P3", false, L"C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( L"C48H18F84N3O6P3", false, L"C2F3O2", false ) );
                reference_receiver_( adcontrols::MSReference( L"C54H18F96N3O6P3", false, L"C2F3O2", false ) );
            } else if ( endGroup == L"AgilentTOF Mix(+)" ) {
                reference_receiver_( adcontrols::MSReference( L"C5H11NO2",        true, L"H", false, 0.0, 1, L"118.0868" ) );
                reference_receiver_( adcontrols::MSReference( L"C6H18N3O6P3",     true, L"H", false, 0.0, 1, L"322.0486" ) );
                reference_receiver_( adcontrols::MSReference( L"C12H18F12N3O6P3", true, L"H", false, 0.0, 1, L"622.0295" ) );
                reference_receiver_( adcontrols::MSReference( L"C18H18F24N3O6P3", true, L"H", false, 0.0, 1, L"922.0103" ) );
                reference_receiver_( adcontrols::MSReference( L"C24H18F36N3O6P3", true, L"H", false ) );
                reference_receiver_( adcontrols::MSReference( L"C30H18F48N3O6P3", true, L"H", false ) );
                reference_receiver_( adcontrols::MSReference( L"C36H18F60N3O6P3", true, L"H", false ) );
                reference_receiver_( adcontrols::MSReference( L"C42H18F72N3O6P3", true, L"H", false ) );
                reference_receiver_( adcontrols::MSReference( L"C48H18F84N3O6P3", true, L"H", false ) );
                reference_receiver_( adcontrols::MSReference( L"C54H18F96N3O6P3", true, L"H", false ) );
            } else {
                // check if an element
                if ( adcontrols::mol::element element = adcontrols::TableOfElement::instance()->findElement( adportable::utf::to_utf8( endGroup ) ) ) {
                    for ( auto& i: element.isotopes() ) {
                        std::wstring formula = ( boost::wformat(L"%1%%2%") % int( i.mass + 0.3 ) % adportable::utf::to_wstring( element.symbol() ) ).str();
                        std::wstring description = ( boost::wformat(L"%.4f") % i.abundance ).str();
                        bool enable = i.abundance > 0.01;
                        reference_receiver_( adcontrols::MSReference( formula, true, L"", enable, i.mass, 1, description ) );
                    }
                } else {
                    // chemical formula
                    reference_receiver_( adcontrols::MSReference( endGroup, true, adduct ) );
                }
            }
        }
    }
}

