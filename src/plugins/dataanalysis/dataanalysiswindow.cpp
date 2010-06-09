/* Copyright (C) 2010 Toshinobu Hondo, Ph.D.
   Science Liaison Project
   */

#include "dataanalysiswindow.h"

#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

using namespace DataAnalysis::Internal;

DataAnalysisWindow::DataAnalysisWindow( QWidget * parent ) : QWidget(parent)
{
  QBoxLayout * layout = new QVBoxLayout( this );
  layout->addWidget( new QTextEdit( tr("Focus me to activate my context!") ) );
  setWindowTitle( tr("Data Analysis") );
}
