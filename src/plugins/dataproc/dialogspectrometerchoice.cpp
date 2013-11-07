/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "dialogspectrometerchoice.hpp"
#include "ui_dialogspectrometerchoice.h"
#include <adcontrols/massspectrometer.hpp>
#include <QStandardItemModel>

using namespace dataproc;

DialogSpectrometerChoice::DialogSpectrometerChoice(QWidget *parent) :
    QDialog(parent)
    , ui(new Ui::DialogSpectrometerChoice)
    , model_( std::make_shared<QStandardItemModel>() )
{
    ui->setupUi(this);
    
	std::vector< std::wstring > names = adcontrols::MassSpectrometer::get_model_names();

	model_->setColumnCount( 1 );
	model_->setRowCount( names.size() );
    int row = 0;
    for ( auto& name: names ) {
        model_->setItem( row++, 0, new QStandardItem( QString::fromStdWString( name ) ) );
    }

    ui->listView->setModel( model_.get() );
}

DialogSpectrometerChoice::~DialogSpectrometerChoice()
{
    delete ui;
}

void dataproc::DialogSpectrometerChoice::on_buttonBox_accepted()
{

}

void dataproc::DialogSpectrometerChoice::on_buttonBox_rejected()
{

}
