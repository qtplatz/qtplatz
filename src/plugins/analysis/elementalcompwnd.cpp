//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "elementalcompwnd.h"

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <adwidgets/dataplot.h>
#include <adwidgets/axis.h>

using namespace Analysis;
using namespace Analysis::internal;

namespace Analysis {
  namespace internal {
    class ElementalCompWndImpl {
    public:
      ~ElementalCompWndImpl() {}
      ElementalCompWndImpl() : ticPlot_(0)
			     , profileSpectrum_(0)
			     , processedSpectrum_(0) {
      }
      
      adil::ui::Dataplot * ticPlot_;
      adil::ui::Dataplot * profileSpectrum_;
      adil::ui::Dataplot * processedSpectrum_;
      
    };

    /////////
  }
}


ElementalCompWnd::ElementalCompWnd(QWidget *parent) :
    QWidget(parent)
{
  init();
}

void
ElementalCompWnd::init()
{
  pImpl_.reset( new ElementalCompWndImpl );
  Core::MiniSplitter * splitter = new Core::MiniSplitter;
  if ( splitter ) {
    if ( pImpl_->processedSpectrum_ = new adil::ui::Dataplot ) {
      adil::ui::Axis axis = pImpl_->processedSpectrum_->axisX();
      axis.text( L"m/z" );
    }
    //splitter->addWidget( pImpl_->ticPlot_ );
    //splitter->addWidget( pImpl_->profileSpectrum_ );
    splitter->addWidget( pImpl_->processedSpectrum_ );
    splitter->setOrientation( Qt::Vertical );
  }
  
  QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
  toolBarAddingLayout->setMargin(0);
  toolBarAddingLayout->setSpacing(0);
  //toolBarAddingLayout->addWidget( toolBar );
  toolBarAddingLayout->addWidget( splitter );
  //toolBarAddingLayout->addWidget( toolBar2 );
}
