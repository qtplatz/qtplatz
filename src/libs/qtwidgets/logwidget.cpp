// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "logwidget.hpp"
#include "ui_logwidget.h"
#if QT_VERSION >= 0x050000
# include <QtWidgets/QPlainTextEdit>
#else
# include <QtGui/QPlainTextEdit>
#endif


using namespace qtwidgets;

LogWidget::LogWidget(QWidget *parent) : QWidget(parent), ui(new Ui::LogWidget)
{
	ui->setupUi(this);
}

LogWidget::~LogWidget()
{
	delete ui;
}

void
LogWidget::OnCreate( const adportable::Configuration& config )
{
    config_ = config;
}

void
LogWidget::OnInitialUpdate()
{
}

void
LogWidget::OnFinalClose()
{
}

void
LogWidget::handle_eventLog( QString str )
{
	ui->plainTextEdit->appendPlainText( str );
}

void
LogWidget::handle_debug_print( unsigned long priority, unsigned long category, QString text )
{
	Q_UNUSED( priority );
    Q_UNUSED( category );
	ui->plainTextEdit->appendPlainText( text );
}

void
LogWidget::getLifeCycle( adplugin::LifeCycle *& p )
{
	p = this;
}
