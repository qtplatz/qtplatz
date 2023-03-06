/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "create_widget.hpp"
#include "ionreactionform.hpp"
#include "utilities.hpp"
#include "spin_t.hpp"
#include "radiobuttonshelper.hpp"
#include <adcontrols/constants.hpp>
#include <adcontrols/ionreactionmethod.hpp>
#include <adportable/debug.hpp>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>

using namespace adwidgets;

namespace adwidgets {

    class IonReactionForm::impl {
    public:
        QGridLayout * gridLayout_2;
        QLabel * label;
        QCheckBox * cbxLowMass;
        QLabel * label_2;
        QSpinBox * spinBoxChargeMin;
        QSpinBox * spinBoxChargeMax;
        QDialogButtonBox * buttonBox;
        std::tuple< QRadioButton *, QRadioButton * > radioButtons;
        impl() : gridLayout_2( 0 )
               , label( 0 )            // 1
               , label_2( 0 )          // 3
               , spinBoxChargeMin( 0 ) // 4
               , spinBoxChargeMax( 0 ) // 6
               , buttonBox( 0 ) {
        }

        void setupUi( IonReactionForm * form ) {
            using namespace spin_initializer;

            auto vLayout = create_widget< QVBoxLayout >( "virticalLayout", form );
            auto hLayout = create_widget< QHBoxLayout >( "horizontalLayout" );
            gridLayout_2 = create_widget< QGridLayout >( "gridLayout_2" );
            gridLayout_2->setContentsMargins(2, 0, 2, 0);
            gridLayout_2->setVerticalSpacing(0);
            vLayout->addLayout( hLayout );
            hLayout->addLayout( gridLayout_2 );

            std::tuple< size_t, size_t > xy{0,0};

            ++xy;
            if (( label = add_widget( gridLayout_2, create_widget< QLabel >( "label", "Min. Charge" )
                                      , std::get<0>(xy), std::get<1>(xy)++ ) )) {
                label->setTextFormat(Qt::RichText);
                label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
            }

            if (( spinBoxChargeMin = add_widget( gridLayout_2, create_widget< QSpinBox >( "spinBoxChargeMin" )
                                                 , std::get<0>(xy), std::get<1>(xy)++ ) )) {
                spin_init( spinBoxChargeMin, std::make_tuple( Minimum<>{1}, Maximum<>{100}, Alignment{Qt::AlignRight} ) );
            }

            ++xy;
            if (( label_2 = add_widget( gridLayout_2, create_widget< QLabel >( "label", "Max. Charge" )
                                        , std::get<0>(xy), std::get<1>(xy)++ ) )) {
                label_2->setTextFormat(Qt::RichText);
                label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
            }

            if (( spinBoxChargeMax = add_widget( gridLayout_2, create_widget< QSpinBox >( "spinBoxChargeMax" )
                                                 , std::get<0>(xy), std::get<1>(xy)++ ) )) {
                spin_init( spinBoxChargeMax, std::make_tuple( Minimum<>{1}, Maximum<>{100}, Alignment{Qt::AlignRight} ) );
            }

            ++xy;
            if ( auto groupBox = add_widget( gridLayout_2, create_widget< QGroupBox >( "GroupBox", tr("Polarity" ) )
                                             , std::get<0>(xy), std::get<1>(xy)++, 1, 2 )){
                auto layout = create_widget< QHBoxLayout >( "groupBox_Layout" );
                layout->setSpacing( 2 );
                layout->setContentsMargins(4, 0, 4, 0);
                std::get<0>(radioButtons) = add_widget( layout, create_widget< QRadioButton >("radioPos", tr("Positive ion") ) );
                std::get<1>(radioButtons) = add_widget( layout, create_widget< QRadioButton >("radioNeg", tr("Negative ion") ) );
                groupBox->setLayout( layout );
            }

            ++xy;
            hLayout->addSpacerItem( new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
            vLayout->addSpacerItem( new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding) );

            if (( buttonBox = add_widget( vLayout, create_widget< QDialogButtonBox >( "buttonBox" ) ) )) {
                buttonBox->setStandardButtons(QDialogButtonBox::Apply);
            }
        }
    };
}

IonReactionForm::IonReactionForm(QWidget *parent) : QWidget(parent)
                                                , impl_( std::make_unique< impl >() )
{
    using namespace spin_initializer;
    impl_->setupUi( this );

    spin_init( impl_->spinBoxChargeMin, std::make_tuple( Minimum<>{1}, Maximum<>{50}, Value<>{1} ) );
    spin_init( impl_->spinBoxChargeMax, std::make_tuple( Minimum<>{1}, Maximum<>{50}, Value<>{3} ) );

    connect( impl_->buttonBox, &QDialogButtonBox::clicked, [this] () { emit triggerProcess(); } );
    connect( std::get<0>(impl_->radioButtons), &QRadioButton::toggled, this, [&]( bool checked ){
        emit polarityToggled( checked ? adcontrols::polarity_positive : adcontrols::polarity_negative );
    });
}

IonReactionForm::~IonReactionForm()
{
}

void
IonReactionForm::getContents( adcontrols::IonReactionMethod& m )
{
    adcontrols::ion_polarity polarity( adcontrols::polarity_positive );
    if ( auto radio = findChild< QRadioButton * >( "radioPos" ) ) {
        polarity = radio->isChecked() ? adcontrols::polarity_positive : adcontrols::polarity_negative;
        if ( radio->isEnabled() )
            m.set_polarity( polarity );
    }
    ADDEBUG() << polarity;
    ADDEBUG() << std::make_pair( impl_->spinBoxChargeMin->value(), impl_->spinBoxChargeMax->value() );
    m.chargeState( { impl_->spinBoxChargeMin->value(), impl_->spinBoxChargeMax->value() }, polarity );
}

void
IonReactionForm::setContents( const adcontrols::IonReactionMethod& m )
{
    adcontrols::ion_polarity polarity( adcontrols::polarity_positive );
    if ( auto radio = findChild< QRadioButton * >( "radioPos" ) ) {
        polarity = radio->isChecked() ? adcontrols::polarity_positive : adcontrols::polarity_negative;
    }
    auto charge = m.chargeState( polarity );
    impl_->spinBoxChargeMin->setValue( charge.first );
    impl_->spinBoxChargeMax->setValue( charge.second );
}

void
IonReactionForm::setPolarity( adcontrols::ion_polarity pol, bool enable )
{
    std::get<0>( impl_->radioButtons )->setChecked( pol == adcontrols::polarity_positive );
    std::get<1>( impl_->radioButtons )->setChecked( pol == adcontrols::polarity_negative );
    if ( auto radio = findChild< QRadioButton * >( "radioPos" ) ) {
        radio->setEnabled( enable );
    }
    if ( auto radio = findChild< QRadioButton * >( "radioNeg" ) ) {
        radio->setEnabled( enable );
    }
    // hide apply button
    impl_->buttonBox->setVisible( enable );
}
