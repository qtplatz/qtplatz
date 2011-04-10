// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <adwplot/dataplot.h>
#include <adwplot/traces.h>
#include <adwplot/trace.h>

#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_scale_engine.h>
#include <qwt_picker_machine.h>

#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>

#include <cmath>

class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QwtPlotCanvas *canvas):
        QwtPlotZoomer(xAxis, yAxis, canvas)
    {
        setTrackerMode(QwtPicker::AlwaysOff);
        setRubberBand(QwtPicker::NoRubberBand);

        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size

        setMousePattern(QwtEventPattern::MouseSelect2,  Qt::RightButton, Qt::ControlModifier);
        setMousePattern(QwtEventPattern::MouseSelect3,  Qt::RightButton);
    }
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    plot_ = new adwplot::Dataplot( this );

    setContextMenuPolicy( Qt::NoContextMenu );
    setCentralWidget( plot_ );
    plot_->setTitle( "Title 1\nTitle 2" );

    QwtPlotZoomer * zoomer = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, plot_->canvas() );
    zoomer->setRubberBand(QwtPicker::RectRubberBand);
    zoomer->setRubberBandPen(QColor(Qt::green));
    zoomer->setTrackerMode(QwtPicker::ActiveOnly);
    zoomer->setTrackerPen(QColor(Qt::white));

    QwtPlotZoomer * zoomer2 = new Zoomer(QwtPlot::xTop, QwtPlot::yRight, plot_->canvas());
    
    QwtPanner * panner = new QwtPlotPanner( plot_->canvas() );
    panner->setMouseButton( Qt::LeftButton, Qt::AltModifier );

    QwtPicker * picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, plot_->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());
    picker->setRubberBandPen(QColor(Qt::green));
    picker->setRubberBand(QwtPicker::CrossRubberBand);
    picker->setTrackerPen(QColor(Qt::white));

 // axes
    plot_->enableAxis(QwtPlot::yRight);
    plot_->setAxisTitle(QwtPlot::xBottom, "Normalized Frequency");
    plot_->setAxisTitle(QwtPlot::yLeft, "Amplitude [dB]");
    plot_->setAxisTitle(QwtPlot::yRight, "Phase [deg]");

    plot_->setAxisMaxMajor(QwtPlot::xBottom, 6);
    plot_->setAxisMaxMinor(QwtPlot::xBottom, 10);
    plot_->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);

// curves
    adwplot::Traces traces = plot_->traces();
    
    adwplot::Trace trace = traces.addTrace( L"Amplitte" );

    std::vector<double> x, y;
    for ( int i = 0; i < 100; ++i ) {
        x.push_back( i + 100 );
        y.push_back( std::sqrt( double(i) ) );
    }
    trace.setData( &x[0], &y[0], x.size() );

/*
    QwtPlotCurve * curve = new QwtPlotCurve( "Amplitute" );
    curve->setRenderHint( QwtPlotItem::RenderAntialiased );
    curve->setPen( QPen( Qt::blue) );
    curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    curve->attach( plot_ );
  */  
}

MainWindow::~MainWindow()
{
}
