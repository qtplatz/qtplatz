/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "thresholdactionform.hpp"
#include "ui_thresholdactionform.h"
#include <adcontrols/threshold_action.hpp>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSignalBlocker>

namespace adwidgets {
}

using namespace adwidgets;

ThresholdActionForm::ThresholdActionForm(QWidget *parent) : QWidget(parent)
                                                      , ui( new Ui::ThresholdActionForm )
{
    ui->setupUi(this);
    connect( ui->groupBox, &QGroupBox::toggled, [this] (bool) { emit valueChanged(); } );
    connect( ui->checkBox, &QCheckBox::toggled, [this] (bool) { emit valueChanged(); } );
    connect( ui->checkBox_2, &QCheckBox::toggled, [this] (bool) { emit valueChanged(); } );
    connect( ui->checkBox_3, &QCheckBox::toggled, [this] (bool) { emit valueChanged(); } );
    connect( ui->doubleSpinBox, static_cast< void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this] (double) { emit valueChanged(); } );
    connect( ui->doubleSpinBox_2, static_cast< void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this] (double) { emit valueChanged(); } );
}

ThresholdActionForm::~ThresholdActionForm()
{
    delete ui;
}

void
ThresholdActionForm::OnCreate( const adportable::Configuration& )
{
}

void
ThresholdActionForm::OnInitialUpdate()
{
}

void
ThresholdActionForm::OnFinalClose()
{
}

void
ThresholdActionForm::onUpdate( boost::any& )
{
}

bool
ThresholdActionForm::getContents( boost::any& ) const
{
    Q_ASSERT( 0 );
    return false;
}

bool
ThresholdActionForm::setContents( boost::any&& )
{
    Q_ASSERT( 0 );    
    return false;
}

bool
ThresholdActionForm::get( adcontrols::threshold_action& m ) const
{
    m.enable = ui->groupBox->isChecked();
    m.recordOnFile = ui->checkBox->isChecked();
    m.enableTimeRange = ui->checkBox_3->isChecked();
    m.exclusiveDisplay = ui->checkBox_2->isChecked();
    m.delay = ui->doubleSpinBox->value() * 1.0e-6;
    m.width = ui->doubleSpinBox_2->value() * 1.0e-6;
    return true;
}

bool
ThresholdActionForm::set( const adcontrols::threshold_action& m )
{
    QSignalBlocker blocks[] = { QSignalBlocker( ui->groupBox ), QSignalBlocker( ui->checkBox ), QSignalBlocker( ui->checkBox_2 )
        , QSignalBlocker( ui->checkBox_3 ), QSignalBlocker( ui->doubleSpinBox ), QSignalBlocker( ui->doubleSpinBox_2 ) };

    ui->checkBox->setChecked( m.enable );
    ui->checkBox_2->setChecked( m.exclusiveDisplay );
    ui->checkBox_3->setChecked( m.recordOnFile );
    ui->doubleSpinBox->setValue( m.delay * 1.0e6 );
    ui->doubleSpinBox_2->setValue( m.width * 1.0e6 );

    return true;
}

