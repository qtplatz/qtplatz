/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "surfacewnd.hpp"
#include "sessionmanager.hpp"
#include "mainwindow.hpp"
#include "dataprocessworker.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/spectrogram.hpp>
#include <adlog/logger.hpp>
#include <adplot/spectrogramwidget.hpp>
#include <adplot/spectrogramdata.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/folder.hpp>
#include <adwidgets/mspeaktable.hpp>
#include <adwidgets/mslockdialog.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qwt_plot_renderer.h>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QScreen>
#include <QtWidgets/QSlider>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QMenu>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QSplitter>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/format.hpp>
#include <algorithm>

using namespace QtDataVisualization;

using namespace dataproc;

SurfaceWnd::SurfaceWnd(QWidget *parent) : QWidget(parent)
{
    Q3DSurface *graph = new Q3DSurface();
    QWidget *container = QWidget::createWindowContainer(graph);

    if (!graph->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
        return;
    }
#if 0
    QSize screenSize = graph->screen()->size();
    container->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.6));
    container->setMaximumSize(screenSize);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);

    //QWidget *widget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    QVBoxLayout *vLayout = new QVBoxLayout();
    hLayout->addWidget(container, 1);
    hLayout->addLayout(vLayout);
    vLayout->setAlignment(Qt::AlignTop);
#endif
    init();
}

void
SurfaceWnd::init()
{
    // if ( QSplitter * splitter = new QSplitter ) {

    //     splitter->addWidget( plot_.get() );
    //     splitter->setOrientation( Qt::Vertical );

    //     if ( QSplitter * v_splitter = new QSplitter ) {

    //         sp_->setMinimumHeight( 40 );
    //         chromatogr_->setMinimumHeight( 40 );

    //         v_splitter->addWidget( sp_.get() );
    //         v_splitter->addWidget( chromatogr_.get() );
    //         v_splitter->setOrientation( Qt::Horizontal );

    //         splitter->addWidget( v_splitter );
    //     }
    //     splitter->setStretchFactor( 0, 9 );
    //     splitter->setStretchFactor( 1, 3 );

    //     QBoxLayout * layout = new QVBoxLayout( this );
    //     layout->addWidget( splitter );
    // }
    setStyleSheet( "background-color: rgb(24,0,0); color: green;" );
}

void
SurfaceWnd::handlePrintCurrentView( const QString& pdfname )
{
	// A4 := 210mm x 297mm (8.27 x 11.69 inch)
#if 0
    QPrinter printer( QPrinter::HighResolution );

    printer.setOutputFileName( pdfname );
    printer.setOrientation( QPrinter::Landscape );
    printer.setDocName( "QtPlatz Spectrogram" );
    printer.setColorMode( QPrinter::Color );
    printer.setPaperSize( QPrinter::A4 );
    printer.setFullPage( false );
    printer.setResolution( 300 );

    QPainter painter( &printer );
    QRectF drawRect( printer.resolution()/2, printer.resolution()/2, printer.width() - printer.resolution(), (12.0/72)*printer.resolution() );

    QRectF boundingRect;
    printer.setDocName( "QtPlatz Process Report" );
    painter.drawText( drawRect, Qt::TextWordWrap, fullpath_.c_str(), &boundingRect );

    drawRect.setTop( boundingRect.bottom() + printer.resolution() / 4 );
    drawRect.setHeight( printer.height() - boundingRect.top() - printer.resolution()/2 );
    // drawRect.setWidth( printer.width() );

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );

    if ( printer.colorMode() == QPrinter::GrayScale )
        renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );

    QRectF rc1( drawRect );
    rc1.setHeight( drawRect.height() * 0.60 );
    renderer.render( plot_.get(), &painter, rc1 );

    QRectF rc2( drawRect );
    rc2.setTop( rc1.bottom() + printer.resolution() / 4 );
	rc2.setHeight( drawRect.height() * 0.30 );
    rc2.setRight( drawRect.width() / 2 );
    renderer.render( sp_.get(), &painter, rc2 );

    rc2.moveLeft( rc2.right() + printer.resolution() / 4 );
    renderer.render( chromatogr_.get(), &painter, rc2 );
#endif
}

void
SurfaceWnd::handleSessionAdded( Dataprocessor* )
{
}

void
SurfaceWnd::handleProcessed( Dataprocessor* processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
SurfaceWnd::handleSelectionChanged( Dataprocessor*, portfolio::Folium& folium )
{
    portfolio::Folder folder = folium.parentFolder();
}

void
SurfaceWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
SurfaceWnd::handleCheckStateChanged( Dataprocessor*, portfolio::Folium&, bool )
{
}

void
SurfaceWnd::handleSelected( const QPointF& pos )
{
    double w = 0.001;
    QRectF rect( QPointF( pos.x(), pos.y() - (w/2) ), QPointF( pos.x(), pos.y() + (w/2) ) );
    handleSelected( rect );
}

void
SurfaceWnd::handleSelected( const QRectF& rect )
{
    QMenu menu;
    std::vector < std::pair< QAction *, std::function<void()> > > actions;
}
