/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include "eventform.hpp"
#include "ui_eventform.h"
#include "document.hpp"

using namespace eventtool;

EventForm::EventForm(QWidget *parent) :
    QWidget(parent)
    , ui( new Ui::EventForm )
    , host_( "localhost" )
    , port_( "7125" )
    , recvPort_( 7125 )
{
    ui->setupUi(this);
    ui->editHost->setText( host_ );
    ui->editPort->setText( port_ );
    ui->spinPort->setMaximum( 65535 );
    ui->spinPort->setValue( recvPort_ );
    ui->doubleSpinBox->setValue( 1.0 );
    ui->checkBoxRepeat->setChecked( Qt::Unchecked );
    
}

EventForm::~EventForm()
{
    delete ui;
}

void EventForm::on_pushButton_clicked()
{
    // bind required if destination host is not the localhost
    if ( (ui->editHost->text() != host_) ||
         (ui->editPort->text() != port_) ) {
        host_ = ui->editHost->text();
        port_ = ui->editPort->text();
        document::instance()->inject_bind( host_.toStdString(), port_.toStdString() );
    }
    document::instance()->inject_event_out();
    last_inject_ = std::chrono::steady_clock::now();
}

void
eventtool::EventForm::on_checkBox_clicked( bool checked )
{
    if ( checked ) { // enable
        short port = ui->spinPort->value();
        if ( document::instance()->monitor_port( port ) ) {
            recvPort_ = port;
        }
        else {
            ui->spinPort->setValue( recvPort_ );
        }
    }
    else {
        document::instance()->monitor_disable();
    }
}

void
eventtool::EventForm::handle_timeout()
{
    if ( ui->checkBoxRepeat->isChecked() ) {
        double minutes = ui->doubleSpinBox->value();
        auto duration = std::chrono::steady_clock::now() - last_inject_;
        if ( std::chrono::duration_cast<std::chrono::minutes>( duration ).count() >= minutes )
            on_pushButton_clicked();
    }
}