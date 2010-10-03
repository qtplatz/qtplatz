//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "outputwindow.h"
#include <QtGui/QStackedWidget>
#include <QtGui/QPlainTextEdit>
#include <qtwrapper/qstring.h>

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

