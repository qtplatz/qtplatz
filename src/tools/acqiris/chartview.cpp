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
#include <QGesture>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QRubberBand>
#include <QLineSeries>
#include <ratio>

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
                                        , data_( 0 )
{
    setRubberBand( QChartView::RectangleRubberBand );
    setRenderHint( QPainter::Antialiasing );

    chart()->addSeries( new QLineSeries() );
    
    // chart()->setTitle("Click to interact with scatter points");

    // m_scatter = new QScatterSeries();
    // m_scatter->setName("scatter1");
    // for (qreal x(0.5); x <= 4.0; x += 0.5) {
    //     for (qreal y(0.5); y <= 4.0; y += 0.5) {
    //         *m_scatter << QPointF(x, y);
    //     }
    // }
    // m_scatter2 = new QScatterSeries();
    // m_scatter2->setName("scatter2");

    // chart()->addSeries(m_scatter2);
    // chart()->addSeries(m_scatter);
    chart()->createDefaultAxes();
    // chart()->axisX()->setRange(0, 4.5);
    // chart()->axisY()->setRange(0, 4.5);

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
ChartView::setData( std::shared_ptr< const waveform > d )
{
    QVector< QPointF > points( d->size() );
    int i(0);
    std::transform( d->begin(), d->end(), points.begin()
                    , [&]( int16_t v ){ return QPointF( d->time( i++ ) * std::micro::den, d->toVolts( v ) ); } );

    auto list = chart()->series();
    if ( list.isEmpty() ) {
        chart()->addSeries( new QLineSeries() );
        list = chart()->series();
    }

    auto series = qobject_cast< QXYSeries * >( list.at( 0 ) );
    
    series->replace( points );
}
