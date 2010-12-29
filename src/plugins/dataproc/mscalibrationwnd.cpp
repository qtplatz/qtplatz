/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "mscalibrationwnd.h"
#include "dataprocessor.h"
#include <portfolio/folium.h>
#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <adwidgets/dataplot.h>
#include <adwidgets/axis.h>

using namespace dataproc;
using namespace dataproc::internal;

namespace dataproc {
    namespace internal {
        class MSCalibrationWndImpl {
        public:
            ~MSCalibrationWndImpl() {}
            MSCalibrationWndImpl() : ticPlot_(0)
                                  , profileSpectrum_(0)
                                  , processedSpectrum_(0) {
            }

            adwidgets::ui::Dataplot * ticPlot_;
            adwidgets::ui::Dataplot * profileSpectrum_;
            adwidgets::ui::Dataplot * processedSpectrum_;

        };
    }
}


MSCalibrationWnd::MSCalibrationWnd(QWidget *parent) :
    QWidget(parent)
{
    init();
}

void
MSCalibrationWnd::init()
{
    pImpl_.reset( new MSCalibrationWndImpl );
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
                if ( pImpl_->processedSpectrum_ = new adwidgets::ui::Dataplot ) {
                        adwidgets::ui::Axis axis = pImpl_->processedSpectrum_->axisX();
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

void
MSCalibrationWnd::handleSessionAdded( Dataprocessor * )
{
}

void
MSCalibrationWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    Q_UNUSED(processor);
    Q_UNUSED(folium);
}
