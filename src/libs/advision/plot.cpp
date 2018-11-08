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

#include "plot.hpp"
#include <QPainter>

using namespace advision;

class QPaintEvent;

plot::plot( QWidget * parent ) : QWidget( parent )
                               , mat_( cv::Mat() )
{
}

plot::~plot()
{
}

QSize
plot::sizeHint() const
{
    return qimg_.size();
}

QSize
plot::minimumSizeHint() const
{
    return qimg_.size();
}

void
plot::show( const cv::Mat& image )
{
    constexpr int Scale = 8;
    switch( image.type() ) {
    case CV_8UC1:
        cv::cvtColor( image, mat_, cv::COLOR_GRAY2RGB );
        break;
    case CV_8UC3:
        cv::cvtColor( image, mat_, cv::COLOR_BGR2RGB );
        break;
    }

    //assert( mat_.isContinuous() );
    qimg_ = QImage( static_cast< const unsigned char *>(mat_.data), mat_.cols, mat_.rows, mat_.step, QImage::Format_RGB888 );
    qimg_ = qimg_.scaled( 64 * Scale, 64 * Scale, Qt::KeepAspectRatio );

    setFixedSize( image.cols * Scale, image.rows * Scale);

    repaint();
}


void
plot::paintEvent( QPaintEvent * )
{
    QPainter painter(this);
    painter.drawImage( QPoint(0,0), qimg_ );
    painter.end();
}

void
plot::setData( const cv::Mat& m )
{
    show( m );
}
