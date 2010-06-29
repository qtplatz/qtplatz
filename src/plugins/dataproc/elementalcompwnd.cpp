//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "elementalcompwnd.h"

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <adwidgets/chromatogramwidget.h>
#include <adwidgets/spectrumwidget.h>
#include <adwidgets/axis.h>

using namespace dataproc::internal;

namespace dataproc {
  namespace internal {
    class ElementalCompWndImpl {
    public:
      ~ElementalCompWndImpl() {}
      ElementalCompWndImpl() : ticPlot_(0)
			     , profileSpectrum_(0)
			     , processedSpectrum_(0) {
      }
      
      adil::ui::ChromatogramWidget * ticPlot_;
      adil::ui::SpectrumWidget * profileSpectrum_;
      adil::ui::SpectrumWidget * processedSpectrum_;
      
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
      if ( pImpl_->processedSpectrum_ = new adil::ui::SpectrumWidget ) {
          splitter->addWidget( pImpl_->processedSpectrum_ );
          splitter->setOrientation( Qt::Vertical );
      }
  }
  
  QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
  toolBarAddingLayout->setMargin(0);
  toolBarAddingLayout->setSpacing(0);
  //toolBarAddingLayout->addWidget( toolBar );
  toolBarAddingLayout->addWidget( splitter );
  //toolBarAddingLayout->addWidget( toolBar2 );
}
