/**************************************************************************
** Copyright (C) 2010-     Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "hostaddrdialog.hpp"
#include "ui_hostaddrdialog.h"
#include <QUrl>

using namespace adwidgets;

HostAddrDialog::HostAddrDialog(QWidget *parent) : QDialog(parent)
                                                , ui( new Ui::HostAddrDialog )
{
    ui->setupUi(this);

    connect( ui->buttonBox, &QDialogButtonBox::accepted, this, [&](){ QDialog::accept(); } );
    connect( ui->buttonBox, &QDialogButtonBox::rejected, this, [&](){ QDialog::reject(); } );
}

HostAddrDialog::~HostAddrDialog()
{
    delete ui;
}

void
HostAddrDialog::setHostAddr( const QString& host, const QString& port )
{
    ui->lineEditHost->setText( host );
    ui->lineEditPort->setText( port );
}

std::pair< QString, QString >
HostAddrDialog::hostAddr() const
{
    return std::make_pair( ui->lineEditHost->text(), ui->lineEditPort->text() );
}

void
HostAddrDialog::setUrl( const QString& addr )
{
    {
        QUrl url( addr );
        ui->lineEditHost->setText( url.host() );
        // ui->lineEditPort->setText( QString::number( url.port() ) );
    }
    
    std::string url = addr.toStdString();
    auto pos = url.find( "http://" );
    if ( pos != std::string::npos )
        pos += 7;
    auto colon_pos = url.find_first_of( ':', pos );
    if ( colon_pos != std::string::npos ) {
        std::string port = url.substr( colon_pos + 1 );
        ui->lineEditPort->setText( QString::fromStdString( port ) );
    }
}
