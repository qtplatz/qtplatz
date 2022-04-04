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

#include "mschromatogramform.hpp"
#include "utilities.hpp"
// #include "ui_mschromatogramform.h"
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/is_type.hpp>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace adwidgets {
    namespace Ui {
        class MSChromatogramForm {
        public:
            QCheckBox *checkBox;
            QDialogButtonBox *buttonBox;
            QDoubleSpinBox *doubleSpinBox;
            QDoubleSpinBox *doubleSpinBox_2;
            QDoubleSpinBox *doubleSpinBox_4;
            //QGridLayout *gridLayout;
            //QGridLayout *gridLayout_2;
            //QGridLayout *gridLayout_3;
            //QGridLayout *gridLayout_4;
            //QGridLayout *gridLayout_5;
            QGroupBox *groupBox;
            QGroupBox *groupBoxAutoTargeting;
            QLabel *label;
            QLabel *label_2;
            QLabel *label_3;
            QLineEdit *lineEdit;
            QRadioButton *radioButton;
            QRadioButton *radioButton_2;
            // QSpacerItem *verticalSpacer;
            QSpinBox *spinBox;
            // QVBoxLayout *verticalLayout;

            void setupUi(QWidget * MSChromatogramForm );
            void retranslateUi(QWidget *MSChromatogramForm );
        };
    } // namespace Ui
}

using namespace adwidgets;

MSChromatogramForm::MSChromatogramForm( QWidget *parent ) : QWidget( parent )
                                                          , ui( new Ui::MSChromatogramForm )
{
    ui->setupUi(this);

    // ui->comboBox->addItems( QStringList() << tr( "pkd.1.u5303a" ) << tr( "1.u5303a" ) );
    connect( ui->checkBox, &QCheckBox::stateChanged, this, [this] ( int state ) { emit onEnableLockMass( state == Qt::Checked ); } );
    connect( ui->buttonBox, &QDialogButtonBox::clicked, [this] () { emit triggerProcess(); } );
}

MSChromatogramForm::~MSChromatogramForm()
{
    delete ui;
}

void
MSChromatogramForm::OnCreate( const adportable::Configuration& )
{
}

void
MSChromatogramForm::OnInitialUpdate()
{
    setContents( adcontrols::MSChromatogramMethod() );
    if ( auto radio = findChild< QRadioButton * >( "radioPos" ) ) {
        connect( radio, &QRadioButton::toggled, [&](bool checked){
            emit polarityToggled( checked ? adcontrols::polarity_positive : adcontrols::polarity_negative );
        });

    }
}

void
MSChromatogramForm::OnFinalClose()
{
}

bool
MSChromatogramForm::getContents( boost::any& any ) const
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( any ) ) {
        if ( adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any ) ) {
            adcontrols::MSChromatogramMethod m;
            getContents( m );
            pm->appendMethod< adcontrols::MSChromatogramMethod >( m );
            return true;
        }
    }
    return false;
}

bool
MSChromatogramForm::setContents( boost::any&& any )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( any ) ) {
        const adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( any );
        if ( const auto t = pm.find< adcontrols::MSChromatogramMethod >() ) {
            setContents( *t );
            return true;
        }
    }
    return false;
}

void
MSChromatogramForm::setContents( const adcontrols::MSChromatogramMethod& m )
{
    // ui->comboBox->setCurrentIndex( QString::fromStdString( m.dataReader() ) );
    ui->lineEdit->setText( QString::fromStdString( m.dataReader() ) );

    if ( m.widthMethod() == adcontrols::MSChromatogramMethod::widthInDa )
        ui->radioButton->setChecked( true );
    else
        ui->radioButton_2->setChecked( true );

    ui->doubleSpinBox->setValue( m.width( adcontrols::MSChromatogramMethod::widthInDa ) );
    ui->spinBox->setValue( m.width( adcontrols::MSChromatogramMethod::widthInRP ) );

    ui->doubleSpinBox_2->setValue( m.peakWidthForChromatogram() );

    ui->checkBox->setChecked( m.lockmass() );

    ui->doubleSpinBox_4->setValue( m.tolerance() * 1000.0 );

    ui->groupBoxAutoTargeting->setChecked( m.enableAutoTargeting() );
}

void
MSChromatogramForm::getContents( adcontrols::MSChromatogramMethod& m ) const
{
    //m.dataSource( static_cast<adcontrols::MSChromatogramMethod::DataSource>(ui->comboBox->currentIndex()) );
    m.setDataReader( ui->lineEdit->text().toStdString() );
    if ( ui->radioButton->isChecked() )
        m.widthMethod( adcontrols::MSChromatogramMethod::widthInDa );
    else
        m.widthMethod( adcontrols::MSChromatogramMethod::widthInRP );

    m.width( ui->doubleSpinBox->value(), adcontrols::MSChromatogramMethod::widthInDa );
    m.width( ui->spinBox->value(), adcontrols::MSChromatogramMethod::widthInRP );

    //m.lower_limit( ui->doubleSpinBox_2->value() );
    //m.upper_limit( ui->doubleSpinBox_3->value() );

    m.setLockmass( ui->checkBox->isChecked() );

    m.setTolerance( ui->doubleSpinBox_4->value() / 1000.0 );

    m.setEnableAutoTargeting( ui->groupBoxAutoTargeting->isChecked() );

    m.setPeakWidthForChromatogram( ui->doubleSpinBox_2->value() );
}

namespace adwidgets {
    namespace Ui {

        void
        MSChromatogramForm::setupUi(QWidget *form )
        {
            if (form->objectName().isEmpty())
                form->setObjectName(QString::fromUtf8("Ui_MSChromatogramForm"));

            if ( auto verticalLayout = create_widget< QVBoxLayout >("verticalLayout", form ) ) {

                groupBox = add_widget( verticalLayout, create_widget< QGroupBox >("groupBox") );

                if ( auto gridLayout = create_widget< QGridLayout >( "gridLayout", groupBox ) ) {
                    gridLayout->setContentsMargins(2, 2, 2, 2);
                    gridLayout->setSpacing(0);

                    std::tuple< size_t, size_t > xy{0,0};

                    label = add_widget( gridLayout, create_widget< QLabel >("label", "Data reader", groupBox ), std::get<0>(xy), std::get<1>(xy)++, 1, 1 );
                    if (( lineEdit = add_widget( gridLayout, create_widget< QLineEdit >("lineEdit", groupBox), std::get<0>(xy), std::get<1>(xy)++, 1, 1 ) )) {
                        lineEdit->setReadOnly(false);
                        lineEdit->setClearButtonEnabled(true);
                    }

                    ++xy; // line 1
                    label_2 = add_widget( gridLayout, create_widget< QLabel >("label_2", "Mass window", groupBox ), std::get<0>(xy), std::get<1>(xy)++, 1, 2 );

                    ++xy; // line 2
                    if (( radioButton = add_widget( gridLayout, create_widget< QRadioButton >("radioButton", groupBox), std::get<0>(xy), std::get<1>(xy)++, 1, 1) )) {
                        radioButton->setChecked(true);
                    }
                    if (( doubleSpinBox = add_widget( gridLayout, create_widget< QDoubleSpinBox >("doubleSpinBox", groupBox), std::get<0>(xy), std::get<1>(xy)++, 1, 1 ) )) {
                        doubleSpinBox->setDecimals(4);
                        doubleSpinBox->setRange( 0.0, 1.0 );
                        doubleSpinBox->setSingleStep(0.001000000000000);
                    }

                    ++xy; // line 3
                    radioButton_2 = add_widget( gridLayout, create_widget< QRadioButton >("radioButton_2", groupBox ), std::get<0>(xy), std::get<1>(xy)++, 1, 1);
                    radioButton_2->setEnabled(true);

                    if (( spinBox = add_widget( gridLayout, create_widget< QSpinBox >("spinBox", groupBox ), std::get<0>(xy), std::get<1>(xy)++, 1, 1) )) {
                        spinBox->setRange(100, 10000000);
                        spinBox->setSingleStep(1000);
                        spinBox->setValue(3000);
                    }

                    ++xy; // line 4
                    checkBox = add_widget( gridLayout, create_widget< QCheckBox >("checkBox", groupBox ), std::get<0>(xy), std::get<1>(xy)++, 1, 1);
                    if (( doubleSpinBox_4 = add_widget( gridLayout
                                                        , create_widget< QDoubleSpinBox >("doubleSpinBox_4", groupBox )
                                                        , std::get<0>(xy), std::get<1>(xy)++, 1, 1) )) {
                        doubleSpinBox_4->setMaximum(1000.000000000000000);
                    }

                    ++xy;
                    if ( auto groupBox = add_widget( gridLayout, create_widget< QGroupBox >( "GroupBox"/*, QObject::tr("Polarity"*/ )
                                                     , std::get<0>(xy), std::get<1>(xy)++, 1, 2 )) {
                        auto layout = create_widget< QHBoxLayout >( "groupBox_Layout" );
                        layout->setSpacing( 2 );
                        layout->setContentsMargins(4, 0, 4, 0);
                        add_widget( layout, create_widget< QRadioButton >( "radioPos", QObject::tr("Positive ion") ) )->setChecked( true );
                        add_widget( layout, create_widget< QRadioButton >( "radioNeg", QObject::tr("Negative ion") ) );

                        groupBox->setLayout( layout );
                    }
                }


                if (( groupBoxAutoTargeting = add_widget( verticalLayout, create_widget< QGroupBox >("groupBoxAutoTargeting" ) ) )) {
                    groupBoxAutoTargeting->setCheckable(true);
                    groupBoxAutoTargeting->setChecked(false);

                    if ( auto gridLayout_4 = create_widget< QGridLayout >("gridLayout_4", groupBoxAutoTargeting ) ) {
                        gridLayout_4->setContentsMargins(4, 0, 4, 0);
                        label_3 = add_widget( gridLayout_4, create_widget< QLabel >("label_3", groupBoxAutoTargeting), 0, 0, 1, 1);

                        if (( doubleSpinBox_2 = add_widget( gridLayout_4, create_widget< QDoubleSpinBox >("doubleSpinBox_2", groupBoxAutoTargeting ), 0, 1, 1, 1 ) )) {
                            doubleSpinBox_2->setDecimals(3);
                            doubleSpinBox_2->setRange(0.001000000000000, 10.000000000000000);
                            doubleSpinBox_2->setSingleStep(0.100000000000000);
                            doubleSpinBox_2->setValue(2.000000000000000);
                        }
                    }
                }

                verticalLayout->addItem( new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding) );
                if (( buttonBox = add_widget( verticalLayout, create_widget< QDialogButtonBox >("buttonBox", form ) ) )) {
                    buttonBox->setStandardButtons(QDialogButtonBox::Apply);
                }
            }

            retranslateUi(form);
            QMetaObject::connectSlotsByName(form);
        } // setupUi

        void
        MSChromatogramForm::retranslateUi(QWidget *form)
        {
            form->setWindowTitle(QCoreApplication::translate("adwidgets::MSChromatogramForm", "Form", nullptr));

            groupBox->setTitle(QCoreApplication::translate("adwidgets::MSChromatogramForm", "Chromatogram generation", nullptr));
            label->setText(QCoreApplication::translate("adwidgets::MSChromatogramForm", "Data reader", nullptr));
            radioButton->setText(QCoreApplication::translate("adwidgets::MSChromatogramForm", "Dalton", nullptr));
            doubleSpinBox_4->setSuffix(QString());
            label_2->setText(QCoreApplication::translate("adwidgets::MSChromatogramForm", "Mass window", nullptr));
            checkBox->setText(QCoreApplication::translate("adwidgets::MSChromatogramForm", "Lockmass (mDa)", nullptr));
            radioButton_2->setText(QCoreApplication::translate("adwidgets::MSChromatogramForm", "R. P.", nullptr));
            groupBoxAutoTargeting->setTitle(QCoreApplication::translate("adwidgets::MSChromatogramForm", "Auto targeting from chromatogram", nullptr));
            label_3->setText(QCoreApplication::translate("adwidgets::MSChromatogramForm", "Peak width (s)", nullptr));
        } // retranslateUi

    } // namespace Ui
} // namespace adwidgets
