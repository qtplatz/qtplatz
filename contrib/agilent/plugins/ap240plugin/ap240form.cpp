/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "ap240form.hpp"
#include "ui_ap240form.h"
#include "ap240methodwidget.hpp"
#include <adinterface/controlserver.hpp>

using namespace ap240;

ap240Form::ap240Form(QWidget *parent) :  QWidget(parent)
                                        , ui(new Ui::ap240Form)
{
    ui->setupUi(this);
}

ap240Form::~ap240Form()
{
    delete ui;
}

void
ap240Form::onInitialUpdate()
{
	ui->pushButton->setEnabled( false );
}

void
ap240Form::onStatus( int status )
{
    if ( status >= controlserver::eStandBy )
        ui->pushButton->setEnabled( true );
}

void
ap240Form::on_pushButton_clicked()
{
    emit trigger_apply();
}
