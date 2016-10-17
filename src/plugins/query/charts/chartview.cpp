/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
**************************************************************************/

#include "chartview.hpp"
#include "waveform.hpp"
#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QScatterSeries>
#include <QVXYModelMapper>
#include <QXYSeries>
#include <QRubberBand>

#include <QAbstractItemModel>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QGesture>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QSvgGenerator>
#include <ratio>

using namespace query::charts;

class Chart : public QChart {
public:
    explicit Chart(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0)
        : QChart( QChart::ChartTypeCartesian, parent, wFlags ) {

        grabGesture(Qt::PanGesture);
        grabGesture(Qt::PinchGesture);
    }
    
    ~Chart() {
    }

protected:
    bool sceneEvent(QEvent *event) {
        if (event->type() == QEvent::Gesture)
            return gestureEvent(static_cast<QGestureEvent *>(event));
        return QChart::event(event);            
    }
        
private:
    bool gestureEvent(QGestureEvent *event) {
        if ( QGesture *gesture = event->gesture(Qt::PanGesture) ) {
            QPanGesture *pan = static_cast<QPanGesture *>(gesture);
            QChart::scroll(-(pan->delta().x()), pan->delta().y());
        }
            
        if ( QGesture *gesture = event->gesture(Qt::PinchGesture) ) {
            QPinchGesture *pinch = static_cast<QPinchGesture *>(gesture);
            if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged)
                QChart::zoom(pinch->scaleFactor());
        }
        return true;            
    }
        
private:
        
};    

////////////////

ChartView::ChartView( QWidget *parent ) : QChartView( new QChart(), parent )
{
    setRubberBand( QChartView::RectangleRubberBand );
    setRenderHint( QPainter::Antialiasing );

    // connect(m_scatter, SIGNAL(clicked(QPointF)), this, SLOT(handleClickedPoint(QPointF)));
}

ChartView::~ChartView()
{
}

// void
// ChartView::handleClickedPoint(const QPointF &point)
// {
// }

bool
ChartView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin) {
        // By default touch events are converted to mouse events. So
        // after this event we will get a mouse event also but we want
        // to handle touch events as gestures only. So we need this safeguard
        // to block mouse events that are actually generated from touch.
        isTouching_ = true;

        // Turn off animations when handling gestures they
        // will only slow us down.
        chart()->setAnimationOptions(QChart::NoAnimation);
    }
    return QChartView::viewportEvent(event);
}

void
ChartView::mousePressEvent(QMouseEvent *event)
{
    if (isTouching_)
        return;
    QChartView::mousePressEvent(event);
}

void
ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if (isTouching_)
        return;
    
    QChartView::mouseMoveEvent(event);
}

void
ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (isTouching_)
        isTouching_ = false;

    // Because we disabled animations when touch event was detected
    // we must put them back on.
    chart()->setAnimationOptions(QChart::SeriesAnimations);
    
    if ( event->button() & Qt::RightButton ) {
        QMenu menu;
        menu.addAction( "Unzoom" );
        menu.addAction( "Copy" );
        menu.addAction( "Copy SVG" );
        menu.addAction( "Save as SVG File..." );

        if ( auto selected = menu.exec( event->globalPos() ) ) {
            if ( selected->text() == "Unzoom" ) {
                chart()->zoomReset();
            } else if ( selected->text() == "Copy" ) {
                copyToClipboard();
            } else if ( selected->text() == "Copy SVG" ) {
                saveImage( true );
            } else if ( selected->text() == "Save as SVG File..." ) {
                saveImage( false );                
            }
            
            return;
        }
    }

    QChartView::mouseReleaseEvent(event);
}

void
ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_Left:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

void
ChartView::clear()
{
    auto chart = this->chart();
    chart->removeAllSeries();    
}

void
ChartView::setData( QAbstractItemModel * model, const QString& title, int x, int y
                    , const QString& xLabel, const QString& yLabel, const QString& chartType )
{
    auto chart = this->chart();

    if ( model ) {
        
        if ( chartType == "Scatter" ) {

            auto series = new QScatterSeries;
            series->setName( title );
            auto mapper = new QVXYModelMapper( this );
            mapper->setXColumn( x );
            mapper->setYColumn( y );
            mapper->setSeries( series );
            mapper->setModel( model );
            chart->addSeries( series );
            chart->createDefaultAxes();

        } else if ( chartType == "Line" ) {
            auto series = new QLineSeries;
            series->setName( title );
            auto mapper = new QVXYModelMapper( this );
            mapper->setXColumn( x );
            mapper->setYColumn( y );
            mapper->setSeries( series );
            mapper->setModel( model );
            chart->addSeries( series );
            chart->createDefaultAxes();

        } else if ( chartType == "Histogram" ) {

            auto series = new QLineSeries;
            int row = 0;
            do {
                model->fetchMore( model->index( row, x ) );
                
                while ( model->index( row, x ).isValid() ) {
                    QPointF p0( model->index( row, x ).data().toDouble(), 0.0 );
                    QPointF p1( model->index( row, x ).data().toDouble(), model->index( row, y ).data().toDouble() );
                    series->append( p0 );
                    series->append( p1 );
                    series->append( p0 );
                    ++row;
                }

            } while ( model->canFetchMore( model->index( row, x ) ) );
                
            series->setName( title );
            chart->addSeries( series );
            chart->createDefaultAxes();

        }
        if ( auto axisX = chart->axisX() )
            axisX->setTitleText( xLabel );
        
        if ( auto axisY = chart->axisY() )
            axisY->setTitleText( yLabel );            
    }
}

void
ChartView::copyToClipboard()
{
    auto pixmap = grab();
    QApplication::clipboard()->setPixmap( pixmap );
}

void
ChartView::saveImage( bool clipboard )
{
    QSvgGenerator generator;
    QByteArray svg;
    QBuffer buffer( &svg );
    
    generator.setTitle( "QtPlatz Generated SVG" );
    generator.setDescription( "Copyright (C) 2013-2017 MS-Cheminformataics, All rights reserved" );
    auto sz = this->size();
    QRectF rc( 0, 0, sz.width(), sz.height() );
    generator.setViewBox( rc );

    if ( clipboard ) {

        generator.setOutputDevice( &buffer );        

    } else {
        auto name = QFileDialog::getSaveFileName( this, tr( "Save SVG File" )
                                                  , "chart.svg"
                                                  , tr( "SVG (*.svg)" ) );
        if ( ! name.isEmpty() )
            generator.setFileName( name );
    }

    QPainter painter;
    painter.begin( &generator );
    painter.setRenderHint( QPainter::Antialiasing );
    render( &painter );
    painter.end();

    if ( clipboard ) {
        QMimeData * mime = new QMimeData();
        mime->setData( "image/svg+xml", svg );
        QApplication::clipboard()->setMimeData( mime, QClipboard::Clipboard );
    }
}
