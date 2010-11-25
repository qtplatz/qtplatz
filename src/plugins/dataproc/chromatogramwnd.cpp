//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "chromatogramwnd.h"
#include "dataprocessor.h"
#include <adcontrols/description.h>
#include <adcontrols/datafile.h>
#include <adcontrols/lcmsdataset.h>
#include <adcontrols/chromatogram.h>
#include <qtwrapper/qstring.h>

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <adwidgets/chromatogramwidget.h>
#include <adwidgets/spectrumwidget.h>
#include <adwidgets/axis.h>
#include <QTableWidget>

using namespace dataproc;
using namespace dataproc::internal;

namespace dataproc {

    namespace internal {
        class ChromatogramWndImpl {
        public:
            ~ChromatogramWndImpl() {}
            ChromatogramWndImpl() : ticPlot_(0)
                , profileSpectrum_(0)
                , processedSpectrum_(0) {
            }

            void setData( const adcontrols::Chromatogram&, const QString& );
      
            adwidgets::ui::ChromatogramWidget * ticPlot_;
            adwidgets::ui::ChromatogramWidget * profileSpectrum_;
            adwidgets::ui::SpectrumWidget * processedSpectrum_;
      
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
        if ( pImpl_->ticPlot_ = new adwidgets::ui::ChromatogramWidget ) {
            splitter->addWidget( pImpl_->ticPlot_ );
            //splitter->addWidget( pImpl_->profileSpectrum_ );
            //splitter->addWidget( pImpl_->processedSpectrum_ );
            QTableWidget * pTable = new QTableWidget;
            pTable->setRowCount(10);
            pTable->setColumnCount(10);
            splitter->addWidget( pTable );
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

void
ChromatogramWnd::handleSessionAdded( Dataprocessor * processor )
{
    adcontrols::datafile& file = processor->file();
    QString filename( qtwrapper::qstring::copy( file.filename() ) );
    adcontrols::LCMSDataset * dset = processor->getLCMSDataset();
    if ( dset ) {
        adcontrols::Chromatogram c;
        if ( dset->getTIC( 0, c ) ) {
            c.addDescription( adcontrols::Description( L"filename", file.filename() ) );
            pImpl_->setData( c, filename );
        }
    }
}

///////////////////////////

void
ChromatogramWndImpl::setData( const adcontrols::Chromatogram& c, const QString& )
{
    ticPlot_->setData( c );
}