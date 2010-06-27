//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "outputwindow.h"
#include <QtGui/QPlainTextEdit>
#include <QtGui/QScrollBar>

using namespace App::internal;

OutputWindow::OutputWindow(QWidget *parent) : Core::IOutputPane(parent)
											, textEdit_(0)
{
  textEdit_ = new QPlainTextEdit();
  if ( textEdit_ ) {
	textEdit_->setWindowTitle( name() );
	//textEdit_->setWindowIcon(":/adbroker/images/xyz.png")
	textEdit_->setReadOnly(true);
	textEdit_->setFrameStyle( QFrame::NoFrame );
/*
	Aggregation::Aggregate * agg = new Aggregation::Aggregate;
	agg->add( textEdit_ );
	agg->add( new Find::BaseTextFind( textEdit_ ) );
*/
  }

}

QWidget *
OutputWindow::outputWidget( QWidget * )
{
  return textEdit_;
}

int
OutputWindow::priorityInStatusBar() const
{
  return 50;
}

void
OutputWindow::clearContents()
{
  textEdit_->clear();
}

void
OutputWindow::visibilityChanged(bool visible)
{
  if ( visible )
	textEdit_->verticalScrollBar()->setValue(textEdit_->verticalScrollBar()->maximum());
}

void
OutputWindow::appendText( const QString& text )
{
  textEdit_->appendHtml( text );
}

bool
OutputWindow::canFocus()
{
  return true;
}

bool
OutputWindow::hasFocus()
{
  return textEdit_->hasFocus();
}

void
OutputWindow::setFocus()
{
  textEdit_->setFocus();
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
