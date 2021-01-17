// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "outputwindow.hpp"

# include <QtWidgets/QStackedWidget>
# include <QtWidgets/QPlainTextEdit>
# include <QtWidgets/QTextEdit>

using namespace servant;

OutputWindow::OutputWindow(void)
{
    widget_ = new QStackedWidget;
    widget_->setWindowTitle( "Output" );

    //textWidget_ = new QTextEdit; // new QPlainTextEdit;
    textWidget_ = new QPlainTextEdit;
	textWidget_->setMaximumBlockCount( 5000 );
    widget_->addWidget( textWidget_ );
}

OutputWindow::~OutputWindow(void)
{
}

void
OutputWindow::appendLog( const std::wstring& text )
{
    appendLog( QString::fromStdWString( text ) );
}

void
OutputWindow::appendLog( const QString& text )
{
	textWidget_->appendPlainText( text );
}

void
OutputWindow::handleLogging( const QString& text, bool richText )
{
    if ( richText )
        textWidget_->appendHtml( text );
    else
        textWidget_->appendPlainText( text );
}

QWidget *
OutputWindow::outputWidget( QWidget * )
{
    return widget_;
}

QList<QWidget *>
OutputWindow::toolBarWidgets() const
{
    return QList<QWidget *>();
}

int
OutputWindow::priorityInStatusBar() const
{
    return 85;
}

void
OutputWindow::visibilityChanged(bool visible)
{
    Q_UNUSED(visible);
}

void
OutputWindow::setFocus()
{
}

void
OutputWindow::goToNext()
{
}

void
OutputWindow::goToPrev()
{
}

void
OutputWindow::clearContents()
{
}
