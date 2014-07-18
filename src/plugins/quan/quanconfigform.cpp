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

namespace quan {
    namespace detail {
        enum idItem { 
            idGroupBox1
            , idRadioChromatogram
            , idRadioInfusion
            , idGroupBox2
            , idLabelCalibEq
            , idRadioEq1Point
            , idRadioEqLinear_0
            , idRadioEqLinear
            , idRadioEqPolynomials
            , idComboPolynomials
            , idCbxWaiting
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
                case idRadioEq1Point: return ui_->radioButton_3;
                case idRadioEqLinear_0:return ui_->radioButton_4;
                case idRadioEqLinear: return ui_->radioButton_5;
                case idRadioEqPolynomials: return ui_->radioButton_6;
                case idComboPolynomials: return ui_->comboBox;
                case idCbxWaiting: return ui_->groupBox_5;
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
                case idSpinLevel: ui_->spinBox;
                case idSpinReplicates: ui_->spinBox_2;
                case idCbxBracketing: ui_->checkBox;
                case idComboBracketing: ui_->comboBox_2;
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
}

QuanConfigForm::~QuanConfigForm()
{
    delete ui;
}

void
QuanConfigForm::setData( std::shared_ptr< adcontrols::QuanMethod >& ptr )
{
    method_ = ptr;
}


void
QuanConfigForm::on_pushButton_clicked()
{

}
