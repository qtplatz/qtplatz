/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "peakidtableform.hpp"
#include "ui_peakidtableform.h"
#include <adportable/configuration.hpp>
#include <QStandardItemModel>
#include <QDeclarativeError>
#include <QMessageBox>

using namespace qtwidgets;

PeakIDTableForm::PeakIDTableForm(QWidget *parent) :
    QDeclarativeView(parent),
    ui(new Ui::PeakIDTableForm)
    , pConfig_( new adportable::Configuration )
{
    // ui->setupUi(this);
}

PeakIDTableForm::~PeakIDTableForm()
{
    delete ui;
}

void
PeakIDTableForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
PeakIDTableForm::OnInitialUpdate()
{
    setSource( QUrl( "qrc://files/CentroidMethodDelegate.qml" ) );
    QList< QDeclarativeError > errors = this->errors();
    for ( QList< QDeclarativeError >::const_iterator it = errors.begin(); it != errors.end(); ++it )
        QMessageBox::warning( this, "QDeclarativeError", it->description() );
}

void
PeakIDTableForm::OnFinalClose()
{
}

QSize
PeakIDTableForm::sizeHint() const
{
    return QSize( 300, 250 );
}

void
PeakIDTableForm::getContents( adcontrols::ProcessMethod& )
{
}
