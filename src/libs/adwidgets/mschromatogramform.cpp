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
            QGridLayout *gridLayout_2;
            QDialogButtonBox *buttonBox;
            QVBoxLayout *verticalLayout;
            QGroupBox *groupBox;
            QGridLayout *gridLayout_3;
            QGridLayout *gridLayout;
            QDoubleSpinBox *doubleSpinBox;
            QLabel *label;
            QRadioButton *radioButton;
            QDoubleSpinBox *doubleSpinBox_4;
            QSpinBox *spinBox;
            QLabel *label_2;
            QCheckBox *checkBox;
            QRadioButton *radioButton_2;
            QLineEdit *lineEdit;
            QGroupBox *groupBoxAutoTargeting;
            QGridLayout *gridLayout_5;
            QGridLayout *gridLayout_4;
            QLabel *label_3;
            QDoubleSpinBox *doubleSpinBox_2;
            QSpacerItem *verticalSpacer;

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
    // ui->checkBox_lower->setChecked( m.lower_limit() < 0 ? false : true );
    // ui->checkBox_upper->setChecked( m.upper_limit() < 0 ? false : true );

    // ui->doubleSpinBox_2->setEnabled( ui->checkBox_lower->isChecked() );
    // ui->doubleSpinBox_3->setEnabled( ui->checkBox_upper->isChecked() );

    // ui->doubleSpinBox_2->setValue( m.lower_limit() );
    // ui->doubleSpinBox_3->setValue( m.upper_limit() );

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

            verticalLayout = create_widget< QVBoxLayout >("verticalLayout");
            // verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

            groupBox = add_widget( verticalLayout, create_widget< QGroupBox >("groupBox", form ) );
            // verticalLayout->addWidget(groupBox);
            // groupBox->setObjectName(QString::fromUtf8("groupBox"));

            gridLayout = create_widget< QGridLayout >("gridLayout" );
            // gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
            gridLayout->setSpacing(0);

            label = add_widget( gridLayout, create_widget< QLabel >("label", groupBox ), 0, 0, 1, 1 );
            // gridLayout->addWidget(label, 0, 0, 1, 1);
            // label->setObjectName(QString::fromUtf8("label"));


            // form->resize(318, 274);
            gridLayout_2 = create_widget< QGridLayout >("gridLayout_2", form);
            // gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
            gridLayout_2->setSpacing(0);
            gridLayout_2->setContentsMargins(4, 2, 4, 2);

            if (( buttonBox = add_widget( gridLayout_2, create_widget< QDialogButtonBox >("buttonBox", form), 2, 0, 1, 1 ) )) {
                // gridLayout_2->addWidget(buttonBox, 2, 0, 1, 1);
                // buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
                buttonBox->setStandardButtons(QDialogButtonBox::Apply);
            }


            gridLayout_3 = create_widget< QGridLayout >("gridLayout_3", groupBox );
            gridLayout_3->setSpacing(2);
            // gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
            gridLayout_3->setContentsMargins(2, 2, 2, 2);


            if (( doubleSpinBox = add_widget( gridLayout, create_widget< QDoubleSpinBox >("doubleSpinBox", groupBox), 2, 1, 1, 1 ) )) {
                // doubleSpinBox->setObjectName(QString::fromUtf8("doubleSpinBox"));
                // gridLayout->addWidget(doubleSpinBox, 2, 1, 1, 1);
                doubleSpinBox->setDecimals(4);
                doubleSpinBox->setMinimum(0.000000000000000);
                doubleSpinBox->setMaximum(1.000000000000000);
                doubleSpinBox->setSingleStep(0.001000000000000);
            }



            radioButton = add_widget( gridLayout, create_widget< QRadioButton >("radioButton", groupBox), 2, 0, 1, 1);
            // gridLayout->addWidget(radioButton, 2, 0, 1, 1);
            // radioButton->setObjectName(QString::fromUtf8("radioButton"));
            radioButton->setChecked(true);


            doubleSpinBox_4 = add_widget( gridLayout, create_widget< QDoubleSpinBox >("doubleSpinBox_4", groupBox ), 4, 1, 1, 1);
            // gridLayout->addWidget(doubleSpinBox_4, 4, 1, 1, 1);
            // doubleSpinBox_4->setObjectName(QString::fromUtf8("doubleSpinBox_4"));
            doubleSpinBox_4->setMaximum(1000.000000000000000);



            spinBox = add_widget( gridLayout, create_widget< QSpinBox >("spinBox", groupBox ), 3, 1, 1, 1);
            // gridLayout->addWidget(spinBox, 3, 1, 1, 1);
            // spinBox->setObjectName(QString::fromUtf8("spinBox"));
            spinBox->setMinimum(100);
            spinBox->setMaximum(10000000);
            spinBox->setSingleStep(1000);
            spinBox->setValue(3000);



            label_2 = add_widget( gridLayout, create_widget< QLabel >("label_2", groupBox), 1, 0, 1, 1);
            // label_2->setObjectName(QString::fromUtf8("label_2"));
            // gridLayout->addWidget(label_2, 1, 0, 1, 1);

            checkBox = add_widget( gridLayout, create_widget< QCheckBox >("checkBox", groupBox ), 4, 0, 1, 1);
            // gridLayout->addWidget(checkBox, 4, 0, 1, 1);
            // checkBox->setObjectName(QString::fromUtf8("checkBox"));


            radioButton_2 = add_widget( gridLayout, create_widget< QRadioButton >("radioButton_2", groupBox ), 3, 0, 1, 1);
            // gridLayout->addWidget(radioButton_2, 3, 0, 1, 1);
            // radioButton_2->setObjectName(QString::fromUtf8("radioButton_2"));
            radioButton_2->setEnabled(true);


            lineEdit = add_widget( gridLayout, create_widget< QLineEdit >("lineEdit", groupBox), 0, 1, 1, 1);
            // gridLayout->addWidget(lineEdit, 0, 1, 1, 1);
            // lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
            lineEdit->setReadOnly(false);
            lineEdit->setClearButtonEnabled(true);


            gridLayout_3->addLayout(gridLayout, 0, 0, 1, 1);

            gridLayout_2->addLayout(verticalLayout, 0, 0, 1, 1);

            if (( groupBoxAutoTargeting = add_widget( gridLayout_2, create_widget< QGroupBox >("groupBoxAutoTargeting", form), 1, 0, 1, 1 ) )) {
                // gridLayout_2->addWidget(groupBoxAutoTargeting, 1, 0, 1, 1);
                // groupBoxAutoTargeting->setObjectName(QString::fromUtf8("groupBoxAutoTargeting"));
                groupBoxAutoTargeting->setCheckable(true);
                groupBoxAutoTargeting->setChecked(false);
            }

            gridLayout_5 = create_widget< QGridLayout >("gridLayout_5", groupBoxAutoTargeting);
            // gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
            gridLayout_5->setContentsMargins(2, 2, 2, 2);

            gridLayout_4 = create_widget< QGridLayout >("gridLayout_4");
            // gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));

            label_3 = add_widget( gridLayout_4, create_widget< QLabel >("label_3", groupBoxAutoTargeting), 0, 0, 1, 1);
            // label_3->setObjectName(QString::fromUtf8("label_3"));
            // gridLayout_4->addWidget(label_3, 0, 0, 1, 1);

            if (( doubleSpinBox_2 = add_widget( gridLayout_4, create_widget< QDoubleSpinBox >("doubleSpinBox_2", groupBoxAutoTargeting ), 0, 1, 1, 1 ) )) {
                // doubleSpinBox_2->setObjectName(QString::fromUtf8("doubleSpinBox_2"));
                // gridLayout_4->addWidget(doubleSpinBox_2, 0, 1, 1, 1);
                doubleSpinBox_2->setDecimals(3);
                doubleSpinBox_2->setMinimum(0.001000000000000);
                doubleSpinBox_2->setMaximum(10.000000000000000);
                doubleSpinBox_2->setSingleStep(0.100000000000000);
                doubleSpinBox_2->setValue(2.000000000000000);
            }

            gridLayout_5->addLayout(gridLayout_4, 0, 0, 1, 1);

            verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

            gridLayout_5->addItem(verticalSpacer, 1, 0, 1, 1);

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
