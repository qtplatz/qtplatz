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
#include <portfolio/folder.h>
#include <adwidgets/spectrumwidget.h>
#include <adutils/processeddata.h>
#include <adwidgets/dataplot.h>
#include <adwidgets/axis.h>

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <boost/any.hpp>

using namespace dataproc;
using namespace dataproc::internal;

namespace dataproc {
    namespace internal {
        class MSCalibrationWndImpl {
        public:
            ~MSCalibrationWndImpl() {}
            MSCalibrationWndImpl() : profileSpectrum_(0)
                                   , processedSpectrum_(0) {
            }

            adwidgets::ui::SpectrumWidget * profileSpectrum_;
            adwidgets::ui::SpectrumWidget * processedSpectrum_;

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
        if ( pImpl_->processedSpectrum_ = new adwidgets::ui::SpectrumWidget ) {
            adwidgets::ui::Axis axis = pImpl_->processedSpectrum_->axisX();
            axis.text( L"m/z" );
        }
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

    int nID(0);
    portfolio::Folder folder = folium.getParentFolder();
    if ( folder && folder.name() == L"MSCalibration" ) {
        boost::any& data = folium;
        if ( data.type() == typeid( adutils::MassSpectrumPtr ) ) {
            adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( data );
            pImpl_->processedSpectrum_->setData( *ptr, nID++ );
        }

        portfolio::Folio attachments = folium.attachments();
        for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
            adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( *it );
            pImpl_->processedSpectrum_->setData( *ptr, nID++ );
        }

    }
}
