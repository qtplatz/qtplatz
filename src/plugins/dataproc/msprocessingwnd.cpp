//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "msprocessingwnd.h"
#include <adwidgets/chromatogramwidget.h>
#include <adwidgets/spectrumwidget.h>
#include <adwidgets/axis.h>
#include <coreplugin/minisplitter.h>
#include <QBoxLayout>

using namespace dataproc::internal;

namespace dataproc {
    namespace internal {
        class MSProcessingWndImpl {
        public:
            ~MSProcessingWndImpl() {}
            MSProcessingWndImpl() : ticPlot_(0)
                                  , profileSpectrum_(0)
                                  , processedSpectrum_(0) {
            }

            adil::ui::ChromatogramWidget * ticPlot_;
            adil::ui::SpectrumWidget * profileSpectrum_;
            adil::ui::SpectrumWidget * processedSpectrum_;

        };
    }
}

MSProcessingWnd::MSProcessingWnd(QWidget *parent) :
    QWidget(parent)
{
    init();
}

void
MSProcessingWnd::init()
{
    pImpl_.reset( new MSProcessingWndImpl );
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        if ( pImpl_->ticPlot_ = new adil::ui::ChromatogramWidget ) {
            adil::ui::Axis axis = pImpl_->ticPlot_->axisX();
            axis.text( L"Time(min)" );
        }

        if ( pImpl_->profileSpectrum_ = new adil::ui::SpectrumWidget ) {
            adil::ui::Axis axis = pImpl_->profileSpectrum_->axisX();
            axis.text( L"m/z" );
        }

        if ( pImpl_->processedSpectrum_ = new adil::ui::SpectrumWidget ) {
            adil::ui::Axis axis = pImpl_->processedSpectrum_->axisX();
            axis.text( L"m/z" );
        }
        splitter->addWidget( pImpl_->ticPlot_ );
        splitter->addWidget( pImpl_->profileSpectrum_ );
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
