/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "spectrogramwnd.hpp"
#include <adcontrols/MassSpectra.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <adwplot/spectrogramwidget.hpp>
#include <qwt_plot_renderer.h>
#include <QPrinter>
#include <QPrintDialog>
#include <QBoxLayout>

using namespace dataproc;

SpectrogramWnd::SpectrogramWnd(QWidget *parent) : QWidget(parent)
                                                , plot_( new adwplot::SpectrogramWidget )
{
    init();
}

void
SpectrogramWnd::init()
{
    QBoxLayout * layout = new QVBoxLayout( this );
    layout->addWidget( plot_.get() );
}

void
SpectrogramWnd::handlePrintCurrentView( const QString& pdfname )
{
    QPrinter printer( QPrinter::HighResolution );
    printer.setOrientation( QPrinter::Landscape );
    printer.setOutputFileName( pdfname );
    
    QPrintDialog dialog( &printer );
    if ( dialog.exec() ) {
        QwtPlotRenderer renderer;

        if ( printer.colorMode() == QPrinter::GrayScale ) {
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground );
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground );
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame );
            renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );
        }

        renderer.renderTo( plot_.get(), printer );
    }

}

void
SpectrogramWnd::handleSessionAdded( Dataprocessor* )
{
}

void
SpectrogramWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    portfolio::Folder folder = folium.getParentFolder();

    if ( folder && folder.name() == L"Spectrograms" && folium.name() == L"Spectrogram" ) {
        // todo
    }
}

void
SpectrogramWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
SpectrogramWnd::handleCheckStateChanged( Dataprocessor*, portfolio::Folium&, bool )
{
}

