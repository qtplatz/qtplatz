/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "targetingform.hpp"
#include "utilities.hpp"
#include "spin_t.hpp"
#include "radiobuttonshelper.hpp"
#include <adcontrols/constants.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adcontrols/metidmethod.hpp>
#include <adcontrols/moltable.hpp>
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

// namespace {

//     template<typename Tuple, std::size_t... Is>
//     void setExclusiveCheckedButtonsImpl( Tuple& t, size_t idx, std::index_sequence<Is...> ) {
//         (( (Is == idx)? std::get<Is>(t)->setChecked(true) : std::get<Is>(t)->setChecked(false)),...);
//     }

//     template<typename... Args>
//     void setExclusiveCheckedButtons( std::tuple< Args...> args, size_t idx ) {
//         setExclusiveCheckedButtonsImpl( args, idx, std::index_sequence_for< Args... >{} );
//     }

//     template<typename Tuple, typename... Args, std::size_t... Is>
//     Tuple radioButtonsStateImpl( std::tuple< Args... > args, std::index_sequence<Is...> ) {
//         Tuple r;
//         ((std::get<Is>( r ) = std::get<Is>(args)->isChecked() ),...);
//         return r;
//     }

//     template<typename Tuple, typename... Args>
//     Tuple radioButtonState( std::tuple< Args... > args, Tuple&& ) {
//         return radioButtonsStateImpl<Tuple>( args, std::index_sequence_for< Args... >{} );
//     }
// }

namespace adwidgets {

    class TargetingForm::impl {
    public:
        QGridLayout * gridLayout_2;
        QLabel * label;
        QCheckBox * cbxLowMass;
        QLabel * label_2;
        QSpinBox * spinBoxChargeMin;
        QCheckBox * cbxHighMass;
        QSpinBox * spinBoxChargeMax;
        QDoubleSpinBox * doubleSpinBoxLowMassLimit;
        QDoubleSpinBox * doubleSpinBoxWidth;
        QDoubleSpinBox * doubleSpinBoxHighMassLimit;
        QCheckBox * checkBox;
        QDialogButtonBox * buttonBox;
        std::tuple< QRadioButton *, QRadioButton * > radioButtons;
        impl() : gridLayout_2( 0 )
               , label( 0 )            // 1
               , cbxLowMass( 0 )       // 2
               , label_2( 0 )          // 3
               , spinBoxChargeMin( 0 ) // 4
               , cbxHighMass( 0 )      // 5
               , spinBoxChargeMax( 0 ) // 6
               , doubleSpinBoxLowMassLimit( 0 ) // null 2
               , doubleSpinBoxWidth( 0 )  // 1
               , doubleSpinBoxHighMassLimit( 0 ) // 2
               , checkBox( 0 )            // 3
               , buttonBox( 0 ) {
        }

        void setupUi( TargetingForm * form ) {
            using namespace spin_initializer;

            auto vLayout = create_widget< QVBoxLayout >( "virticalLayout", form );
            auto hLayout = create_widget< QHBoxLayout >( "horizontalLayout" );
            gridLayout_2 = create_widget< QGridLayout >( "gridLayout_2" );
            gridLayout_2->setContentsMargins(2, 0, 2, 0);
            gridLayout_2->setVerticalSpacing(0);
            vLayout->addLayout( hLayout );
            hLayout->addLayout( gridLayout_2 );

            std::tuple< size_t, size_t > xy{0,0};

            if ( auto label = add_widget( gridLayout_2, create_widget< QLabel >( "label_width", "Width (mDa)" )
                                          , std::get<0>(xy), std::get<1>(xy)++ ) ) {
                label->setTextFormat(Qt::RichText);
                label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
            }
            if (( doubleSpinBoxWidth = add_widget( gridLayout_2, create_widget< QDoubleSpinBox >( "doublSpinBoxWidth" )
                                                   , std::get<0>(xy), std::get<1>(xy)++ ) )) {
                spin_init( doubleSpinBoxWidth, std::make_tuple( Decimals{2}, Minimum<>{0.01}, Maximum<>{500.}, Alignment{Qt::AlignRight} ) );
            }

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
            if (( cbxLowMass = add_widget( gridLayout_2, create_widget< QCheckBox >( "cbxLowMass", tr("Low mass limit"))
                                           , std::get<0>(xy), std::get<1>(xy)++ ) )) {
            }
            /////////
            if (( doubleSpinBoxLowMassLimit = add_widget( gridLayout_2, create_widget< QDoubleSpinBox >( "doublSpinBoxLowMassLimit" )
                                                          , std::get<0>(xy), std::get<1>(xy)++ ) )) {
                spin_init( doubleSpinBoxLowMassLimit, std::make_tuple( Minimum<>{0.0}, Maximum<>{10'000}, Decimals{2}, Alignment{Qt::AlignRight} ) );
            }

            ++xy;
            if (( cbxHighMass = add_widget( gridLayout_2, create_widget< QCheckBox >( "cbxHightMass", tr("High mass limit"))
                                            , std::get<0>(xy), std::get<1>(xy)++ ) )) {
            }

            if (( doubleSpinBoxHighMassLimit = add_widget( gridLayout_2, create_widget< QDoubleSpinBox >( "doublSpinBoxHighMassLimit" )
                                                           , std::get<0>(xy), std::get<1>(xy)++ ) )) {
                spin_init( doubleSpinBoxHighMassLimit, std::make_tuple( Minimum<>{0.0}, Maximum<>{10'000}, Decimals{2}, Alignment{Qt::AlignRight} ) );
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
            if (( checkBox = add_widget( gridLayout_2, create_widget< QCheckBox >( "checkBox", tr("Closest m/z"))
                                         , std::get<0>(xy), std::get<1>(xy)++ ) )) {
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

TargetingForm::TargetingForm(QWidget *parent) : QWidget(parent)
                                              , impl_( std::make_unique< impl >() )
                                              , ui( impl_ )
{
    using namespace spin_initializer;
    ui->setupUi( this );

    // spin_t<QSpinBox, int >::init( ui->spinBoxChargeMin, 1, 50, 1 );
    // spin_t<QSpinBox, int >::init( ui->spinBoxChargeMax, 1, 50, 3 );
    spin_init( ui->spinBoxChargeMin, std::make_tuple( Minimum<>{1}, Maximum<>{50}, Value<>{1} ) );
    spin_init( ui->spinBoxChargeMax, std::make_tuple( Minimum<>{1}, Maximum<>{50}, Value<>{3} ) );

	ui->cbxLowMass->setCheckState( Qt::Unchecked );
	ui->cbxHighMass->setCheckState( Qt::Unchecked );
	//spin_t<QDoubleSpinBox, double >::init( ui->doubleSpinBoxLowMassLimit, 1, 5000,  100 );
	//spin_t<QDoubleSpinBox, double >::init( ui->doubleSpinBoxHighMassLimit, 1, 5000, 2000 );
    spin_init( ui->doubleSpinBoxLowMassLimit, std::make_tuple( Minimum<>{1.}, Maximum<>{5000.}, Value<>{100.} ) );
    spin_init( ui->doubleSpinBoxHighMassLimit, std::make_tuple( Minimum<>{1.}, Maximum<>{5000.}, Value<>{4000.} ) );

    connect( ui->buttonBox, &QDialogButtonBox::clicked, [this] () { emit triggerProcess(); } );
    connect( std::get<0>(ui->radioButtons), &QRadioButton::toggled, this, [&]( bool checked ){
        emit polarityToggled( checked ? adcontrols::polarity_positive : adcontrols::polarity_negative );
    });
}

TargetingForm::~TargetingForm()
{
}

void
TargetingForm::setTitle( const QString& title, bool enableCharge, bool enableLimits )
{
    if ( !enableCharge ) {
        ui->cbxLowMass->setCheckState( Qt::Unchecked );
        ui->cbxHighMass->setCheckState( Qt::Unchecked );
        ui->cbxLowMass->setEnabled( false );
        ui->cbxHighMass->setEnabled( false );
    }
    if ( !enableLimits ) {
        ui->doubleSpinBoxLowMassLimit->setEnabled( false );
        ui->doubleSpinBoxHighMassLimit->setEnabled( false );
    }
}

void
TargetingForm::getContents( adcontrols::TargetingMethod& m )
{
    m.setTolerance( adcontrols::idToleranceDaltons, ui->doubleSpinBoxWidth->value() / 1000.0 ); // mDa --> Da
    m.chargeState( ui->spinBoxChargeMin->value(), ui->spinBoxChargeMax->value() );

    m.isLowMassLimitEnabled( ui->cbxLowMass->checkState() == Qt::Checked );
    m.isHighMassLimitEnabled( ui->cbxHighMass->checkState() == Qt::Checked );

	m.lowMassLimit( ui->doubleSpinBoxLowMassLimit->value() );
	m.highMassLimit( ui->doubleSpinBoxHighMassLimit->value() );

    if ( ui->checkBox->isChecked() )
        m.setFindAlgorithm( adcontrols::idFindClosest );
    else
        m.setFindAlgorithm( adcontrols::idFindLargest );

    if ( auto radio = findChild< QRadioButton * >( "radioPos" ) ) {
        m.molecules().setPolarity( radio->isChecked() ? adcontrols::polarity_positive : adcontrols::polarity_negative );
    }
}

void
TargetingForm::setContents( const adcontrols::TargetingMethod& m )
{
    // QSignalBlocker block( this );

    ui->doubleSpinBoxWidth->setValue( m.tolerance( adcontrols::idToleranceDaltons ) * 1000.0 );
    auto charge = m.chargeState();
    ui->spinBoxChargeMin->setValue( charge.first );
    ui->spinBoxChargeMax->setValue( charge.second );

    auto limits = m.isMassLimitsEnabled();

    ui->cbxLowMass->setCheckState( limits.first ? Qt::Checked : Qt::Unchecked );
    ui->cbxHighMass->setCheckState( limits.second ? Qt::Checked : Qt::Unchecked );

	ui->doubleSpinBoxLowMassLimit->setValue( m.lowMassLimit() );
	ui->doubleSpinBoxHighMassLimit->setValue( m.highMassLimit() );

    ui->checkBox->setChecked( m.findAlgorithm() == adcontrols::idFindClosest );

    radiobuttons::setChecked( ui->radioButtons, m.molecules().polarity() );
    // ADDEBUG() << "RadioButtns state: " << radiobuttons::checkedStates( ui->radioButtons, std::tuple<bool,bool>{} );
}

void
TargetingForm::getContents( adcontrols::MetIdMethod& m )
{
    m.setTolerance( adcontrols::idToleranceDaltons, ui->doubleSpinBoxWidth->value() / 1000.0 ); // mDa --> Da
    m.chargeState( { ui->spinBoxChargeMin->value(), ui->spinBoxChargeMax->value() } );

    if ( ui->checkBox->isChecked() )
        m.setFindAlgorithm( adcontrols::idFindClosest );
    else
        m.setFindAlgorithm( adcontrols::idFindLargest );

    radiobuttons::setChecked( ui->radioButtons, m.polarity() );
}

void
TargetingForm::setContents( const adcontrols::MetIdMethod& m )
{
    ui->doubleSpinBoxWidth->setValue( m.tolerance( adcontrols::idToleranceDaltons ) * 1000.0 );
    auto chargeMin = m.chargeState().first;
    auto chargeMax = m.chargeState().second;
    ui->spinBoxChargeMin->setValue( chargeMin );
    ui->spinBoxChargeMax->setValue( chargeMax );
    ui->checkBox->setChecked( m.findAlgorithm() == adcontrols::idFindClosest );

    ui->cbxLowMass->setEnabled( false );
    ui->cbxHighMass->setEnabled( false );

    ui->cbxLowMass->setCheckState( Qt::Unchecked );
    ui->cbxHighMass->setCheckState( Qt::Unchecked );

	ui->doubleSpinBoxLowMassLimit->setValue( 0.0 );
	ui->doubleSpinBoxHighMassLimit->setValue( 4000.0 );
    ui->doubleSpinBoxLowMassLimit->setEnabled( false );
	ui->doubleSpinBoxHighMassLimit->setEnabled( false );

    radiobuttons::setChecked( ui->radioButtons, m.polarity() );
}
