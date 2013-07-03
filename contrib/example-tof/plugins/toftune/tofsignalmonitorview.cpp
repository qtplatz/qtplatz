/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "tofsignalmonitorview.hpp"
#include <tofinterface/signalC.h>

#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <adwplot/tracewidget.hpp>
#include <adportable/fft.hpp>
#include <adportable/array_wrapper.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/minisplitter.h>
#include <utils/styledbar.h>

#include <QTextEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <boost/array.hpp>
#include <complex>
#include <iomanip>

using namespace toftune;


tofSignalMonitorView::tofSignalMonitorView(QWidget *parent) : QWidget(parent)
                                                            , lower_( 0 )
                                                            , upper_( 0 )
                                                            , area_( 0 )
                                                            , height_( 0 )
	                                                        , tplot_( 0 )
															, splot_( 0 )
                                                            , sp1_( 0 )
                                                            , sp2_( 0 )
                                                            , sp3_( 0 )
{
}

tofSignalMonitorView::~tofSignalMonitorView()
{
}

tofSignalMonitorView *
tofSignalMonitorView::Create( QWidget * parent )
{
	Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
    if ( ! splitter3 )
		return 0;

	splitter3->setOrientation( Qt::Vertical );

	tofSignalMonitorView * widget = new tofSignalMonitorView( parent );
	if ( ! widget )
		return 0;

    widget->tplot_ = new adwplot::ChromatogramWidget;

    widget->splot_ = new adwplot::SpectrumWidget;
    widget->sp1_ = new adwplot::SpectrumWidget;
    widget->sp2_ = new adwplot::SpectrumWidget;
    widget->sp3_ = new adwplot::SpectrumWidget;

    widget->tplot_->setMinimumHeight( 100 );
    widget->splot_->setMinimumHeight( 100 );
    widget->sp1_->setMinimumHeight( 100 );
    widget->sp2_->setMinimumHeight( 100 );
    widget->sp3_->setMinimumHeight( 100 );

    do {
        Core::MiniSplitter * splitter = new Core::MiniSplitter;
        splitter->setOrientation( Qt::Horizontal );
        splitter3->addWidget( splitter );

        splitter->addWidget( widget->sp1_ );
        splitter->addWidget( widget->sp2_ );
        splitter->addWidget( widget->sp3_ );
    } while ( 0 );

    do {
        Core::MiniSplitter * splitter = new Core::MiniSplitter;
        splitter->setOrientation( Qt::Horizontal );
        splitter3->addWidget( splitter );
        splitter->addWidget( widget->tplot_ );  // TIC
        splitter->addWidget( widget->splot_ );  // full spectrum
    } while ( 0 );

    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
		toolBar->setProperty( "topBorder", true );
		QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
		toolBarLayout->addWidget( new QLabel( tr("Connect" ) ) );
	}

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( widget );
    toolBarAddingLayout->setMargin( 0 );
	toolBarAddingLayout->setSpacing( 2 );
	toolBarAddingLayout->addWidget( toolBar );
    toolBarAddingLayout->addWidget( splitter3 );

	return widget;
}

void
tofSignalMonitorView::setData( const adcontrols::MassSpectrum& ms )
{
    splot_->setData( ms );
    sp1_->setData( ms );
    sp2_->setData( ms );
    sp3_->setData( ms );
}

void
tofSignalMonitorView::setData( const adcontrols::Trace& trace, const std::wstring& traceId )
{
    (void)traceId;
    tplot_->setData( trace );
}

static double noise( size_t i )
{
    double sampling_frequency = 25000000;  // 25MHz
    double wave_frequency     =  1000000;   // 1MHz
    double a = wave_frequency * (M_PI * 2) * i / sampling_frequency;
    return std::sin( a ) * 0; // 1000;
}
