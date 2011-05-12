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

#include "outputwindow.hpp"
#include <QtGui/QStackedWidget>
#include <QtGui/QPlainTextEdit>
#include <qtwrapper/qstring.hpp>

using namespace servant;

OutputWindow::OutputWindow(void)
{
    widget_ = new QStackedWidget;
    widget_->setWindowTitle( name() );

    textWidget_ = new QPlainTextEdit;
    widget_->addWidget( textWidget_ );
}

OutputWindow::~OutputWindow(void)
{
}

void
OutputWindow::appendLog( const std::wstring& text )
{
    textWidget_->appendPlainText( qtwrapper::qstring::copy( text ) );
}

void
OutputWindow::appendLog( const QString& text )
{
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

bool
OutputWindow::hasFocus()
{
    return false;
}

bool
OutputWindow::canFocus()
{
    return false;
}

void
OutputWindow::setFocus()
{
}

bool
OutputWindow::canNext()
{
    return false;
}

bool
OutputWindow::canPrevious()
{
    return false;
}

void
OutputWindow::goToNext()
{
}

void
OutputWindow::goToPrev()
{
}

bool
OutputWindow::canNavigate()
{
    return false;
}

void
OutputWindow::clearContents()
{
}

