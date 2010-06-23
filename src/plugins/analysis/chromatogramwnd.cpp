//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "chromatogramwnd.h"

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <adwidgets/dataplot.h>
#include <adwidgets/axis.h>
#include <QTableWidget>

using namespace Analysis;
using namespace Analysis::internal;

namespace Analysis {
  namespace internal {
    class ChromatogramWndImpl {
    public:
      ~ChromatogramWndImpl() {}
      ChromatogramWndImpl() : ticPlot_(0)
			     , profileSpectrum_(0)
			     , processedSpectrum_(0) {
      }
      
      adil::ui::Dataplot * ticPlot_;
      adil::ui::Dataplot * profileSpectrum_;
      adil::ui::Dataplot * processedSpectrum_;
      
    };

    //////////
  }
}


ChromatogramWnd::ChromatogramWnd(QWidget *parent) :
    QWidget(parent)
{
  init();
}

void
ChromatogramWnd::init()
{
    pImpl_.reset( new ChromatogramWndImpl );
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
      if ( pImpl_->ticPlot_ = new adil::ui::Dataplot ) {
	adil::ui::Axis axis = pImpl_->ticPlot_->axisX();
	axis.text( L"Time(min)" );
      }
      splitter->addWidget( pImpl_->ticPlot_ );
      //splitter->addWidget( pImpl_->profileSpectrum_ );
      //splitter->addWidget( pImpl_->processedSpectrum_ );
      QTableWidget * pTable = new QTableWidget;
      pTable->setRowCount(10);
      pTable->setColumnCount(10);
      splitter->addWidget( pTable );
      splitter->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    //toolBarAddingLayout->addWidget( toolBar );
    toolBarAddingLayout->addWidget( splitter );
    //toolBarAddingLayout->addWidget( toolBar2 );
}
