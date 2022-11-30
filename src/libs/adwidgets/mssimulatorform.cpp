/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "utilities.hpp"
#include "mssimulatorform.hpp"
#include <adcontrols/constants.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mssimulatormethod.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adportable/debug.hpp>
#include <adutils/constants.hpp> // clsid for massspectrometer

#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpacerItem>
#include <QSpinBox>
#include <QWidget>
#include <QtCore/QVariant>

using namespace adwidgets;

namespace adwidgets {
    namespace Ui {
        class MSSimulatorForm {
        public:
            MSSimulatorForm();
            QComboBox *comboBox;
            QComboBox *comboBox_2;
            QDoubleSpinBox *doubleSpinBox_3;
            QDoubleSpinBox *doubleSpinBox_4;
            QDoubleSpinBox *doubleSpinBox_5;
            QGridLayout *gridLayout;
            QGridLayout *gridLayout_2;
            QGridLayout *gridLayout_3;
            QGroupBox *groupBox;
            QHBoxLayout *horizontalLayout;
            QHBoxLayout *horizontalLayout_2;
            QLabel *label;
            QLabel *label_2;
            QLabel *label_3;
            QLabel *label_4;
            QLabel *label_5;
            QLabel *label_6;
            QPushButton *pushButton;
            QRadioButton *radioButtonNeg;
            QRadioButton *radioButtonPos;
            QSpacerItem *horizontalSpacer;
            QSpinBox *spinBox;
            QSpinBox *spinBox_2;
            QSpinBox *spinBox_3;
            QSpinBox *spinBox_lap;
            QVBoxLayout *verticalLayout;
            QVBoxLayout *verticalLayout_2;
            void setupUi( adwidgets::MSSimulatorForm * );
            void retranslateUi(QWidget *);
        };
    }
}

MSSimulatorForm::MSSimulatorForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MSSimulatorForm)
{
    ui->setupUi(this);

    ui->comboBox->setEnabled( false );
    ui->spinBox_lap->setEnabled( false );
    connect( ui->spinBox,     qOverload< int >( &QSpinBox::valueChanged ), [this]( int ){ emit onValueChanged(); } );
    connect( ui->spinBox_lap, qOverload< int >( &QSpinBox::valueChanged ), [this]( int value ){ emit onValueChanged(); emit onLapChanged( value ); } );

    connect( ui->doubleSpinBox_3, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit onValueChanged(); } );
    connect( ui->doubleSpinBox_4, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit onValueChanged(); } );
    connect( ui->doubleSpinBox_5, static_cast<void( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), [this] ( double ) { emit onValueChanged(); } );

    connect( ui->spinBox_2, qOverload< int >( &QSpinBox::valueChanged )
             , [this] ( int ) {
                   QSignalBlocker block( ui->spinBox_3 );
                   if ( ui->spinBox_3->value() < ui->spinBox_2->value() )
                       ui->spinBox_3->setValue( ui->spinBox_2->value() );
                   emit onValueChanged();
               } );

    connect( ui->spinBox_3, qOverload< int >( &QSpinBox::valueChanged )
             , [this] ( int ) {
                   QSignalBlocker block( ui->spinBox_2 );
                   if ( ui->spinBox_3->value() < ui->spinBox_2->value() )
                       ui->spinBox_2->setValue( ui->spinBox_3->value() );
                   emit onValueChanged();
               } );

    connect( ui->spinBox_3, qOverload< int >( &QSpinBox::valueChanged ), [this] ( int ) { emit onValueChanged(); } );

    connect( ui->groupBox, &QGroupBox::toggled, this, &MSSimulatorForm::onTOFToggled ); // )[this](bool) { emit onValueChanged(); } );
    connect( ui->pushButton, &QPushButton::pressed, [this] () { emit triggerProcess(); } );

    connect( ui->comboBox, qOverload< int >( &QComboBox::currentIndexChanged ), this, [&](int index){
        // ADDEBUG() << ui->comboBox->currentData().toInt();
    });

    ui->comboBox_2->addItems({"0.1", "0.01", "0.001", "1.0e-4", "1.0e-5", "1.0e-6", "1.0e-7", "1.0e-8", "1.0e-9"});
    ui->comboBox_2->setCurrentIndex( 2 );

    if ( auto radioPos = findChild< QRadioButton * >( "radioButtonPos" ) ) {
        connect( radioPos, &QRadioButton::toggled, this, [&]( bool checked ){
            emit polarityToggled( checked ? adcontrols::polarity_positive : adcontrols::polarity_negative );
        });
    }
}

MSSimulatorForm::~MSSimulatorForm()
{
    delete ui;
}

void
MSSimulatorForm::OnInitialUpdate()
{
}

void
MSSimulatorForm::OnFinalClose()
{
}

bool
MSSimulatorForm::getContents( adcontrols::MSSimulatorMethod& m ) const
{
    m.setResolvingPower( ui->spinBox->value() );
    m.setChargeStateMin( ui->spinBox_2->value() );
    m.setChargeStateMax( ui->spinBox_3->value() );
    m.setIsTof( ui->groupBox->isChecked() );
    m.setLength( ui->doubleSpinBox_3->value() );
    m.setAcceleratorVoltage( ui->doubleSpinBox_4->value() );
    m.setTDelay( ui->doubleSpinBox_5->value() * 1.0e-6 );
    m.setIsPositivePolarity( ui->radioButtonPos->isChecked() );

    m.setMode( ui->spinBox_lap->value() );
    m.setProtocol( ui->comboBox->currentIndex() );

    int index = ui->comboBox_2->currentIndex(); // 0.1 .. 1.0e-9
    m.setAbundanceLowLimit( std::pow(10, -(index + 1)) );

    // ui->groupBox->setStyleSheet( "*{fontSize: 9pt}");
    // ui->gridLayout_3->widget()->setHidden( !m.isTof() );
    // ui->groupBox->setHidden( !m.isTof() );

    return true;
}

bool
MSSimulatorForm::setContents( const adcontrols::MSSimulatorMethod& m )
{
    QSignalBlocker blocks( this );

    ui->spinBox->setValue( m.resolvingPower() );

    ui->spinBox_2->setValue( m.chargeStateMin() );
    ui->spinBox_3->setValue( m.chargeStateMax() );
    ui->groupBox->setChecked ( m.isTof() );
    ui->doubleSpinBox_3->setValue( m.length() );
    ui->doubleSpinBox_4->setValue( m.acceleratorVoltage() );
    ui->doubleSpinBox_5->setValue( m.tDelay() * 1.0e6 );

    while ( ui->comboBox->count() <= m.protocol() )
        ui->comboBox->addItem( QString("p%1").arg( ui->comboBox->count() ) );

    ui->comboBox->setCurrentIndex( m.protocol() );
    ui->spinBox_lap->setValue( m.mode() );

    ui->radioButtonPos->setChecked( m.isPositivePolarity() );

    connect( ui->spinBox_lap, qOverload<int>(&QSpinBox::valueChanged), [&](int lap){
        if ( auto sp = massSpectrometer_.lock() ) {
            if ( auto law = sp->scanLaw() )
                ui->doubleSpinBox_3->setValue( law->fLength( lap ) );
        }
    });

    int index = -(std::log10( m.abundanceLowLimit()  ) + 1);
    ui->comboBox_2->setCurrentIndex( index );

    // ADDEBUG() << "limit: " << m.abundanceLowLimit() << " --> index: " << std::pow( 10, m.abundanceLowLimit() );
    return true;
}

void
MSSimulatorForm::setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > p )
{
    QSignalBlocker block( this );
    massSpectrometer_ = p;
    if ( p ) {
        ui->doubleSpinBox_3->setValue( p->fLength() );
        ui->doubleSpinBox_4->setValue( p->acceleratorVoltage() );
        ui->doubleSpinBox_5->setValue( p->tDelay() * std::micro::den );
        if ( p->massSpectrometerClsid() == qtplatz::infitof::iids::uuid_massspectrometer ) {

            if ( auto law = p->scanLaw() )
                ui->doubleSpinBox_3->setValue( law->fLength( 0 ) );

            ui->comboBox->setEnabled( true );
            ui->spinBox_lap->setEnabled( true );
            ui->spinBox_lap->setValue( p->mode( 0 ) );
            ui->doubleSpinBox_4->setEnabled( false );
            ui->doubleSpinBox_5->setEnabled( false );
        } else {
            ui->comboBox->setEnabled( false );
            ui->spinBox_lap->setEnabled( false );
            ui->doubleSpinBox_4->setEnabled( true );
            ui->doubleSpinBox_5->setEnabled( true );
        }
    }
}

void
MSSimulatorForm::setMassSpectrum( std::shared_ptr< const adcontrols::MassSpectrum > p )
{
    massSpectrum_ = p;
    if ( p ) {
        ui->comboBox->clear();
        int proto(0);
        for ( const auto& fms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *p ) )
            ui->comboBox->addItem( QString( "p%1" ).arg( proto++ ), fms.mode() );
        ui->spinBox_lap->setValue( p->mode() );
    }
}

void
MSSimulatorForm::onTOFToggled( bool checked )
{
    emit onValueChanged();
    // ui->groupBox->setHidden( !checked );

#if defined Q_OS_MAC
    static std::pair< QString, QString > styles { "*{ font-size: 8pt;}", "*{ font-size: 12pt;}" };
#else
    static std::pair< QString, QString > styles { "*{ font-size: 7pt;}", "*{ font-size: 9pt;}" };
#endif
    ui->groupBox->setStyleSheet( checked ? std::get<1>(styles) : std::get<0>(styles) );
}

namespace adwidgets {

    namespace Ui {
        MSSimulatorForm::MSSimulatorForm()
            : comboBox( 0 )
            , comboBox_2( 0 )
            , doubleSpinBox_3( 0 )
            , doubleSpinBox_4( 0 )
            , doubleSpinBox_5( 0 )
            , gridLayout( 0 )
            , gridLayout_2( 0 )
            , gridLayout_3( 0 )
            , groupBox( 0 )
            , horizontalLayout( 0 )
            , horizontalLayout_2( 0 )
            , label( 0 )
            , label_2( 0 )
            , label_3( 0 )
            , label_4( 0 )
            , label_5( 0 )
            , label_6( 0 )
            , pushButton( 0 )
            , radioButtonNeg( 0 )
            , radioButtonPos( 0 )
            , horizontalSpacer( 0 )
            , spinBox( 0 )
            , spinBox_2( 0 )
            , spinBox_3( 0 )
            , spinBox_lap( 0 )
            , verticalLayout( 0 )
            , verticalLayout_2( 0 )
        {}

        void
        MSSimulatorForm::setupUi( adwidgets::MSSimulatorForm * form )
        {
            using namespace adwidgets;
            using namespace spin_initializer;

            if (form->objectName().isEmpty())
                form->setObjectName(QString::fromUtf8("form"));

            verticalLayout = create_widget< QVBoxLayout >("verticalLayout", form );
            verticalLayout->setSpacing(2);
            verticalLayout->setContentsMargins(4, 4, 4, 4);

            if (( gridLayout = create_widget< QGridLayout >("gridLayout") )) {
                gridLayout->setVerticalSpacing(0);
                gridLayout->setContentsMargins( 4, 0, 4, 0 );

                std::tuple< size_t, size_t > xy = {0,0};

                // line 0
                if (( label = add_widget( gridLayout, create_widget< QLabel >("label", form), std::get<0>(xy), std::get<1>(xy)++, 1, 1 ) )) {
                    label->setAlignment(Qt::AlignCenter);
                }
                if (( spinBox = add_widget( gridLayout, create_widget< QSpinBox >("spinBox", form), std::get<0>(xy), std::get<1>(xy)++, 1, 1 ) )) {
                    spin_init( spinBox, std::make_tuple( Minimum<>{100}, Maximum<>{10000000}, SingleStep<>{100}, Value<>{10000},Alignment{Qt::AlignRight} ) );
                }

                ++xy; // line 1
                label_6 = add_widget( gridLayout, create_widget< QLabel >("label_6", form), std::get<0>(xy), std::get<1>(xy)++, 1, 1 );
                label_6->setAlignment(Qt::AlignCenter);
                comboBox_2 = add_widget( gridLayout, create_widget< QComboBox >("comboBox_2", form), std::get<0>(xy), std::get<1>(xy)++, 1, 1 );

                ++xy; // line 2
                label_2 = add_widget( gridLayout, create_widget< QLabel >("label_2", form), std::get<0>(xy), std::get<1>(xy)++, 1, 1 );
                label_2->setAlignment(Qt::AlignCenter);

                // line 2
                if (( horizontalLayout = create_widget< QHBoxLayout >("horizontalLayout") )) {
                    spinBox_2 = add_widget( horizontalLayout, create_widget< QSpinBox >("spinBox_2", form) );
                    spinBox_3 = add_widget( horizontalLayout, create_widget< QSpinBox >("spinBox_3", form) );
                    gridLayout->addLayout(horizontalLayout, std::get<0>(xy), std::get<1>(xy)++, 1, 1);
                }

                ++xy; // line 3
                if ( auto gbx = add_widget( gridLayout, create_widget<QGroupBox>( "GroupBoxPolarity" /*, QObject::tr("Polarity")*/ )
                                            , std::get<0>(xy), std::get<1>(xy)++, 1, 2 ) ) {
                    auto _layout = create_widget< QHBoxLayout >("GroupBoxPolarityLayout", gbx );
                    _layout->setSpacing( 0 );
                    _layout->setContentsMargins( 4, 0, 4, 0 );
                    radioButtonPos = add_widget( _layout, create_widget< QRadioButton >("radioButtonPos", form ) ); radioButtonPos->setChecked(true);
                    radioButtonNeg = add_widget( _layout, create_widget< QRadioButton >("radioButtonNeg", form ) );
                }
                ++xy; // end of gridLayout
                verticalLayout->addLayout(gridLayout);
            }

            // TOF
            if (( groupBox = add_widget( verticalLayout, create_widget< QGroupBox >("groupBox", form) ) )) {
                groupBox->setFlat(false);
                groupBox->setCheckable(true);
                if (( gridLayout_3 = create_widget< QGridLayout >( "gridLayout_3", groupBox ) )) {
                    gridLayout_3->setHorizontalSpacing(2);
                    gridLayout_3->setVerticalSpacing(0);
                    gridLayout_3->setContentsMargins(0, 0, 0, 0);
                }
                gridLayout_2 = create_widget< QGridLayout >("gridLayout_2");
                gridLayout_2->setVerticalSpacing(1);

                label_5 = add_widget( gridLayout_2, create_widget< QLabel >( "label_5", groupBox ), 2, 0, 1, 1 );
                label_5->setTextFormat(Qt::RichText);
                label_5->setAlignment(Qt::AlignCenter);

                spinBox_lap = add_widget( gridLayout_2, create_widget< QSpinBox >("spinBox_lap", groupBox), 3, 1, 1, 1 );
                spin_init( spinBox_lap, std::make_tuple( Maximum<>{9999},Alignment{Qt::AlignRight} ) );
                spinBox_lap->setEnabled(false);

                if (( doubleSpinBox_3 = add_widget( gridLayout_2, create_widget< QDoubleSpinBox >("doubleSpinBox_3", groupBox), 0, 1, 1, 1 ) )) {
                    spin_init( doubleSpinBox_3, std::make_tuple( Minimum<>{0.1},Maximum<>{999.99},Value<>{2.0},Alignment{Qt::AlignRight} ) );
                }

                if (( doubleSpinBox_4 = add_widget( gridLayout_2, create_widget< QDoubleSpinBox >("doubleSpinBox_4", groupBox), 1, 1, 1, 1 ) )) {
                    spin_init( doubleSpinBox_4, std::make_tuple( Decimals{1}, Maximum<>{50000.0},Value<>{7000.0},Alignment{Qt::AlignRight} ) );
                }

                if (( doubleSpinBox_5 = add_widget( gridLayout_2, create_widget< QDoubleSpinBox >("doubleSpinBox_5", groupBox), 2, 1, 1, 1 ) )) {
                    spin_init( doubleSpinBox_4, std::make_tuple( Decimals{5}, Minimum<>{-1000.0}, Maximum<>{10000},SingleStep<>{0.01},Alignment{Qt::AlignRight} ) );
                }

                label_3 = add_widget( gridLayout_2, create_widget< QLabel >("label_3", groupBox), 0, 0, 1, 1 );
                label_4 = add_widget( gridLayout_2, create_widget< QLabel >("label_4", groupBox), 1, 0, 1, 1 );
                comboBox = add_widget( gridLayout_2, create_widget< QComboBox >("comboBox", groupBox), 3, 0, 1, 1 );
                comboBox->addItems({ "p0", "p1", "p2", "p3" } );
                gridLayout_3->addLayout(gridLayout_2, 0, 0, 1, 1);
                // verticalLayout->addWidget(groupBox);
            }
            verticalLayout->addSpacerItem( new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding) );
            if ( auto buttonBox = add_widget( verticalLayout, create_widget< QDialogButtonBox >( "buttonBox" ) ) ) {
                buttonBox->setStandardButtons(QDialogButtonBox::Apply);
                pushButton = buttonBox->button(QDialogButtonBox::Apply);
            }

            verticalLayout->setStretch(0, 1);
            verticalLayout->setStretch(1, 1);

            retranslateUi( form );

            comboBox_2->setCurrentIndex(-1);
        }

        void
        MSSimulatorForm::retranslateUi(QWidget * form )  {
            form->setWindowTitle(QCoreApplication::translate("adwidgets::MSSimulatorForm",    "Form", nullptr));
            radioButtonNeg->setText(QCoreApplication::translate("adwidgets::MSSimulatorForm", "NEG", nullptr));
            label_2->setText(QCoreApplication::translate("adwidgets::MSSimulatorForm",        "Charge", nullptr));
            radioButtonPos->setText(QCoreApplication::translate("adwidgets::MSSimulatorForm", "POS", nullptr));
            label->setText(QCoreApplication::translate("adwidgets::MSSimulatorForm",          "R.P.", nullptr));
            label_6->setText(QCoreApplication::translate("adwidgets::MSSimulatorForm",        "RA limits", nullptr));
            groupBox->setTitle(QCoreApplication::translate("adwidgets::MSSimulatorForm",      "TOF", nullptr));
            label_5->setText(QCoreApplication::translate("adwidgets::MSSimulatorForm",        "<i>T<sub>0</sub></it> (<it>&mu;s</i>)", nullptr));
            label_3->setText(QCoreApplication::translate("adwidgets::MSSimulatorForm",        "Length(m)", nullptr));
            label_4->setText(QCoreApplication::translate("adwidgets::MSSimulatorForm",        "Accel. (V)", nullptr));
            // comboBox->setItemText(0, QCoreApplication::translate("adwidgets::MSSimulatorForm", "p0", nullptr));
            // comboBox->setItemText(1, QCoreApplication::translate("adwidgets::MSSimulatorForm", "p1", nullptr));
            // comboBox->setItemText(2, QCoreApplication::translate("adwidgets::MSSimulatorForm", "p2", nullptr));
            // comboBox->setItemText(3, QCoreApplication::translate("adwidgets::MSSimulatorForm", "p3", nullptr));

            pushButton->setText(QCoreApplication::translate("adwidgets::MSSimulatorForm", "Apply", nullptr));
        } // retranslateUi
    }
}
