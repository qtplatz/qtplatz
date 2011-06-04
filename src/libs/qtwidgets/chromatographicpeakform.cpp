// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "chromatographicpeakform.hpp"
#include "ui_chromatographicpeakform.h"
#include <adportable/configuration.hpp>
#include <QStandardItemModel>

using namespace qtwidgets;

ChromatographicPeakForm::ChromatographicPeakForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChromatographicPeakForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )

{
    ui->setupUi(this);
}

ChromatographicPeakForm::~ChromatographicPeakForm()
{
    delete ui;
}

void
ChromatographicPeakForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
ChromatographicPeakForm::OnInitialUpdate()
{
}

void
ChromatographicPeakForm::OnFinalClose()
{
}

QSize
ChromatographicPeakForm::sizeHint() const
{
    return QSize( 300, 250 );
}

void
ChromatographicPeakForm::getContents( adcontrols::ProcessMethod& )
{
}
