/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "countingform.hpp"
#include <adcontrols/targetingmethod.hpp>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QSizePolicy>

namespace adwidgets {

    template<class _Ty,  class... _Types>
    inline QWidget * create_widget(const char * ident, _Types&&... _Args)
    {
        auto w = new _Ty( std::forward<_Types>(_Args)...);
        if ( ident && *ident )
            w->setObjectName( ident );
        return w;
    }

}

using namespace adwidgets;

CountingForm::CountingForm(QWidget *parent) : QWidget(parent)
{
    resize( 280, 80 );
    auto gridLayout = new QGridLayout( this );

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    //sizePolicy.setHeightForWidth(sizePolicy().hasHeightForWidth());
    setSizePolicy(sizePolicy);

    int row(0);
    int col(0);

    gridLayout->addWidget( create_widget< QLabel >( "label1", "Formula" ), row, col++ );
    gridLayout->addWidget( create_widget< QLabel >( "label2", "Time(&mu;s)" ), row, col++ );
    gridLayout->addWidget( create_widget< QLabel >( "label3", "Width(&mu;s)" ), row, col++ );
    ++row;
    col = 0;
    gridLayout->addWidget( create_widget< QLineEdit >( "edit1" ), row, col++ );
    gridLayout->addWidget( create_widget< QDoubleSpinBox >( "edit2" ), row, col++ );
    gridLayout->addWidget( create_widget< QDoubleSpinBox >( "edit3" ), row, col++ );

    // ui->setupUi(this);

    // ui->radioButtonRP->setChecked( false );
    // ui->radioButtonWidth->setChecked( true );    
    // spin_t<QDoubleSpinBox, double>::init( ui->doubleSpinBoxRP, 1000.0, 100000.0, 10000.0 );
    // spin_t<QDoubleSpinBox, double>::init( ui->doubleSpinBoxWidth, 0.1, 500.0, 1.0 );
    // spin_t<QSpinBox, int >::init( ui->spinBoxChargeMin, 1, 50, 1 );
    // spin_t<QSpinBox, int >::init( ui->spinBoxChargeMax, 1, 50, 3 );

	// ui->cbxLowMass->setCheckState( Qt::Unchecked );
	// ui->cbxHighMass->setCheckState( Qt::Unchecked );
	// spin_t<QDoubleSpinBox, double >::init( ui->doubleSpinBoxLowMassLimit, 1, 5000,  100 );
	// spin_t<QDoubleSpinBox, double >::init( ui->doubleSpinBoxHighMassLimit, 1, 5000, 2000 );

    // connect( ui->buttonBox, &QDialogButtonBox::clicked, [this] () { emit triggerProcess(); } );
}

CountingForm::~CountingForm()
{
}

void
CountingForm::setTitle( const QString& title, bool enableCharge, bool enableLimits )
{
    // ui->groupBox->setTitle( title );
    // if ( !enableCharge ) {
    //     ui->cbxLowMass->setCheckState( Qt::Unchecked );
    //     ui->cbxHighMass->setCheckState( Qt::Unchecked );
    //     ui->cbxLowMass->setEnabled( false );
    //     ui->cbxHighMass->setEnabled( false );
    // }
    // if ( !enableLimits ) {
    //     ui->doubleSpinBoxLowMassLimit->setEnabled( false );
    //     ui->doubleSpinBoxHighMassLimit->setEnabled( false );
    // }
}

void
CountingForm::getContents( adcontrols::TargetingMethod& m )
{
    // m.setTolerance( adcontrols::idTolerancePpm, ui->doubleSpinBoxRP->value() );

    // m.setTolerance( adcontrols::idToleranceDaltons, ui->doubleSpinBoxWidth->value() / 1000.0 ); // mDa --> Da
    // m.setToleranceMethod( ui->radioButtonRP->isChecked() ? adcontrols::idTolerancePpm : adcontrols::idToleranceDaltons );

    // m.chargeState( ui->spinBoxChargeMin->value(), ui->spinBoxChargeMax->value() );

    // m.isLowMassLimitEnabled( ui->cbxLowMass->checkState() == Qt::Checked );
    // m.isHighMassLimitEnabled( ui->cbxHighMass->checkState() == Qt::Checked );

	// m.lowMassLimit( ui->doubleSpinBoxLowMassLimit->value() );
	// m.highMassLimit( ui->doubleSpinBoxHighMassLimit->value() );

    // if ( ui->checkBox->isChecked() )
    //     m.setFindAlgorithm( adcontrols::idFindClosest );
    // else
    //     m.setFindAlgorithm( adcontrols::idFindLargest );
}

void
CountingForm::setContents( const adcontrols::TargetingMethod& m )
{
    // ui->doubleSpinBoxRP->setValue( m.tolerance( adcontrols::idTolerancePpm ) );
    // ui->doubleSpinBoxWidth->setValue( m.tolerance( adcontrols::idToleranceDaltons ) * 1000.0 );
    // ui->radioButtonRP->setChecked( m.toleranceMethod() == adcontrols::idTolerancePpm );
    // auto charge = m.chargeState();
    // ui->spinBoxChargeMin->setValue( charge.first );
    // ui->spinBoxChargeMax->setValue( charge.second );

    // auto limits = m.isMassLimitsEnabled();

    // ui->cbxLowMass->setCheckState( limits.first ? Qt::Checked : Qt::Unchecked );
    // ui->cbxHighMass->setCheckState( limits.second ? Qt::Checked : Qt::Unchecked );

	// ui->doubleSpinBoxLowMassLimit->setValue( m.lowMassLimit() );
	// ui->doubleSpinBoxHighMassLimit->setValue( m.highMassLimit() );

    // ui->checkBox->setChecked( m.findAlgorithm() == adcontrols::idFindClosest );
}
