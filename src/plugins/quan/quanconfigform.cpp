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

#include "quanconfigform.hpp"
#include "ui_quanconfigform.h"
#include <qtwrapper/font.hpp>
#include <adcontrols/quanmethod.hpp>
#include <QMessageBox>
#include <algorithm>

namespace quan {
    namespace detail {
        enum idItem { 
            idGroupBox1
            , idRadioChromatogram
            , idRadioInfusion
            , idGroupBox2
            , idLabelCalibEq
            // , idRadioEq1Point
            // , idRadioEqLinear_0
            // , idRadioEqLinear
            // , idRadioEqPolynomials
            , idComboPolynomials
            , idCbxWeighting
            , idRadio_C1
            , idRadio_C2        
            , idRadio_C3
            , idRadio_Y1
            , idRadio_Y2        
            , idRadio_Y3
            , idGroupBox_ISTD
            , idRadioInternalStandard
            , idRadioExternalStandard
            , idGroupBox_Levels
            , idLabelLevel
            , idLabelReplicates
            , idSpinLevel
            , idSpinReplicates
            , idCbxBracketing
            , idComboBracketing
            , idEnd
        };
        struct ui_accessor {
            Ui::QuanConfigForm * ui_;
            ui_accessor( Ui::QuanConfigForm * ui ) : ui_( ui ) {}
            QWidget * operator()( idItem id ) {
                switch ( id ) {
                case idGroupBox1: return ui_->groupBox_2;
                case idRadioChromatogram: return ui_->radioButton;
                case idRadioInfusion: return ui_->radioButton_2;
                case idGroupBox2: return ui_->groupBox;
                case idLabelCalibEq: return ui_->groupBox_6;
                    //case idRadioEq1Point: return ui_->radioButton_3;
                    //case idRadioEqLinear_0:return ui_->radioButton_4;
                    //case idRadioEqLinear: return ui_->radioButton_5;
                    //case idRadioEqPolynomials: return ui_->radioButton_6;
                case idComboPolynomials: return ui_->comboBox;
                case idCbxWeighting: return ui_->groupBox_5;
                case idRadio_C1: return ui_->radioButton_7;
                case idRadio_C2: return ui_->radioButton_8;
                case idRadio_C3: return ui_->radioButton_9;
                case idRadio_Y1: return ui_->radioButton_10;
                case idRadio_Y2: return ui_->radioButton_11;
                case idRadio_Y3: return ui_->radioButton_12;
                case idGroupBox_ISTD: return ui_->groupBox_4;
                case idRadioInternalStandard: return ui_->radioButton_13;
                case idRadioExternalStandard: return ui_->radioButton_14;
                case idGroupBox_Levels: return ui_->groupBox_3;
                case idLabelLevel: return ui_->label;
                case idLabelReplicates: return ui_->label_2;
                case idSpinLevel: return ui_->spinBox;
                case idSpinReplicates: return ui_->spinBox_2;
                case idCbxBracketing: return ui_->checkBox;
                case idComboBracketing: return ui_->comboBox_2;
                case idEnd:
                    break;
                }
                return 0;
            }
        };
    }
}

using namespace quan;
using namespace quan::detail;

QuanConfigForm::QuanConfigForm(QWidget *parent) :  QWidget(parent)
                                                , ui(new Ui::QuanConfigForm)
{
    ui->setupUi(this);

    QFont font;
    qtwrapper::font::setFamily( font, qtwrapper::fontForm );

    ui_accessor accessor( ui );
    for ( int id = idGroupBox1; id < idEnd; ++id ) {
        if ( QWidget * w = accessor( idItem( id ) ) )
            w->setFont( font );
    }
    if ( QComboBox * order = dynamic_cast<QComboBox *>(accessor( idComboPolynomials )) ) {
        order->clear();
        order->insertItems( 0, QStringList()
                            << "1-Point"
                            << "Linear (through 0,0)"
                            << "Linear (regression)"
                            << "Y=a+bX+cX^2"
                            << "Y=a+bX+cX^2+dX^3"
                            << "Y=a+bX+cX^2+dX^3+eX4"
                            << "Y=a+bX+cX^2+dX^3+eX^4+fX^5" );
    }
    if ( QComboBox * combo = dynamic_cast<QComboBox *>(accessor( idComboBracketing )) ) {
        combo->clear();
        combo->insertItems( 0, QStringList() << "None" << "Standard" << "Moving(Overlapped)" << "Average" );
    }
}

QuanConfigForm::~QuanConfigForm()
{
    delete ui;
}

bool
QuanConfigForm::setContents( const adcontrols::QuanMethod& m )
{
    ui_accessor accessor(ui);

    QWidget * w = 0;
    
    if ( auto combo = dynamic_cast<QComboBox *>(accessor( idComboPolynomials )) ) {
        uint32_t order = m.polynomialOrder() - 2;
        switch( m.equation() ) {
        case adcontrols::QuanMethod::idCalibOnePoint:      combo->setCurrentIndex( 0 ); break;
        case adcontrols::QuanMethod::idCalibLinear_origin: combo->setCurrentIndex( 1 ); break;
        case adcontrols::QuanMethod::idCalibLinear:        combo->setCurrentIndex( 2 ); break;
        case adcontrols::QuanMethod::idCalibPolynomials:   combo->setCurrentIndex( 3 + order ); break;
        }
    }

    if ( m.isChromatogram() ) {
        if ( auto radioButton = dynamic_cast< QRadioButton * >( accessor( idRadioChromatogram ) ) )
            radioButton->setChecked( true );
    } else {
        if ( auto radioButton = dynamic_cast< QRadioButton * >( accessor( idRadioInfusion ) ) )
            radioButton->setChecked( true );
    }
    if ( auto gbx = dynamic_cast<QGroupBox *>(accessor( idCbxWeighting )) ) {
        if ( m.isWeighting() )
            gbx->setChecked( true );
        else
            gbx->setChecked( false );
    }
    w = 0;
    switch ( m.weighting() ) {
    case adcontrols::QuanMethod::idWeight_C1: w = accessor( idRadio_C1 ); break;
    case adcontrols::QuanMethod::idWeight_C2: w = accessor( idRadio_C2 ); break;
    case adcontrols::QuanMethod::idWeight_C3: w = accessor( idRadio_C3 ); break;
    case adcontrols::QuanMethod::idWeight_Y1: w = accessor( idRadio_Y1 ); break;
    case adcontrols::QuanMethod::idWeight_Y2: w = accessor( idRadio_Y2 ); break;
    case adcontrols::QuanMethod::idWeight_Y3: w = accessor( idRadio_Y3 ); break;
    }
    if ( auto radioButton = dynamic_cast<QRadioButton *>(w) ) {
        radioButton->setChecked( true );
    }
    if ( m.isInternalStandard() ) {
        if ( auto radioButton = dynamic_cast<QRadioButton *>(accessor( idRadioInternalStandard )) )
            radioButton->setChecked( true );
    } else {
        if ( auto radioButton = dynamic_cast<QRadioButton *>(accessor( idRadioExternalStandard )) )
            radioButton->setChecked( true );
    }
    if ( auto spin = dynamic_cast< QSpinBox *>( accessor(idSpinLevel))) {
        spin->setValue( m.levels());
    }
    if ( auto spin = dynamic_cast< QSpinBox *>( accessor(idSpinReplicates))) {
        spin->setValue( m.replicates() );
    }
    return true;
}

bool
QuanConfigForm::getContents( adcontrols::QuanMethod& m )
{
    ui_accessor accessor( ui );

    if ( auto combo = dynamic_cast<QComboBox *>(accessor( idComboPolynomials )) ) {
        uint32_t idEq = combo->currentIndex();
        if ( idEq == 0 )
            m.equation( adcontrols::QuanMethod::idCalibOnePoint );
        else if ( idEq == 1 )
            m.equation( adcontrols::QuanMethod::idCalibLinear_origin );
        else if ( idEq == 2 )
            m.equation( adcontrols::QuanMethod::idCalibLinear );
        else if ( idEq >= 3 ) {
            m.equation( adcontrols::QuanMethod::idCalibPolynomials );
            m.polynomialOrder( idEq - 3 + 2 );
        }
    }

    if ( auto radioButton = dynamic_cast<QRadioButton *>(accessor( idRadioChromatogram )) )
        if ( radioButton->isChecked() )
            m.isChromatogram( true );
    if ( auto radioButton = dynamic_cast<QRadioButton *>(accessor( idRadioInfusion )) ) {
        if ( radioButton->isChecked() )
            m.isChromatogram( false );
    }
    if ( auto gbx = dynamic_cast<QGroupBox *>(accessor( idCbxWeighting )) ) {
        m.isWeighting( gbx->isChecked() );
    }

    do {
        static const idItem items[] = { idRadio_C1, idRadio_C2, idRadio_C3, idRadio_Y1, idRadio_Y2, idRadio_Y3 };
        for ( auto& id : items ) {
            if ( auto radio = dynamic_cast<QRadioButton *>(accessor( id )) ) {
                if ( radio->isChecked() ) {
                    m.weighting( static_cast<adcontrols::QuanMethod::CalibWeighting>(id - items[ 0 ]) );
                    break;
                }
            }
        }
    } while ( 0 );

    if ( auto radioButton = dynamic_cast<QRadioButton *>(accessor( idRadioInternalStandard )) ) {
        if ( radioButton->isChecked() )
            m.isInternalStandard( true );
        else
            m.isInternalStandard( false );
    }

    if ( auto spin = dynamic_cast< QSpinBox *>( accessor(idSpinLevel))) {
        int value = spin->value();
        m.levels( value );
    }
    if ( auto spin = dynamic_cast< QSpinBox *>( accessor(idSpinReplicates))) {
        m.replicates( spin->value() );
    }
    return true;
}

void
QuanConfigForm::on_pushButton_clicked()
{
    QMessageBox::information(0, "QuanConfigForm", "Sorry, not implemented yet");
}
