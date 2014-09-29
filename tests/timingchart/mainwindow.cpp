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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <adplot/timingchart.hpp>
#include <adplot/spectrumwidget.hpp>
#include <QBoxLayout>

MainWindow::MainWindow(QWidget *parent) :  QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize( 800, 350 );
    auto layout = new QHBoxLayout( centralWidget() );
    auto chart = new adplot::TimingChart;
    layout->addWidget( chart );

    (*chart) << adplot::TimingChart::Pulse( 0, 1.0e-6, "PULSE" )
        << adplot::TimingChart::Pulse( 0, 2.0e-6, "INJECT" )
        << adplot::TimingChart::Pulse( 0, 20.0e-6, "EJECT" );

}

MainWindow::~MainWindow()
{
    delete ui;
}
