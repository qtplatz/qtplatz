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

#include "u5303aform.hpp"
#include "ui_u5303aform.h"
#include "u5303amethodwidget.hpp"
#include <adinterface/controlserver.hpp>

using namespace u5303a;

u5303AForm::u5303AForm(QWidget *parent) :  QWidget(parent)
                                        , ui(new Ui::u5303AForm)
{
    ui->setupUi(this);
}

u5303AForm::~u5303AForm()
{
    delete ui;
}

void
u5303AForm::onInitialUpdate()
{
	ui->pushButton->setEnabled( false );
}

void
u5303AForm::onStatus( int status )
{
    if ( status >= controlserver::eStandBy )
        ui->pushButton->setEnabled( true );
}

void
u5303AForm::on_pushButton_clicked()
{
    emit trigger_apply();
}
