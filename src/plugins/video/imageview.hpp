/**************************************************************************
** Copyright (C) 2016-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <QFrame>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <memory>

class QGraphicsView;
class QLabel;
class QPaintEvent;
class QSlider;
class QToolButton;

namespace video {

    class ImageView : public QFrame {

        Q_OBJECT

    public:
        ImageView( int index, QWidget * parent = 0 );
        ~ImageView();

        QSize sizeHint() const;
        QSize minimumSizeHint() const;

        void setImage( const cv::Mat& );
        void setImage( const QImage& );
        void drawComponents();
        void clearImage();
        void resetButtons();
        void setMaxZ( int );
        int z() const;
        bool isChecked( const QString& ) const;
        int index() const { return index_; }
        QGraphicsView * graphicsView();

    private:
    
    signals:
        void zValue( ImageView *, int );
        void toggled( ImageView *, const QString&, bool );

    public slots:
        void zoomIn(int level = 1);
        void zoomOut(int level = 1);

    private slots:
        void resetView();
        void setResetButtonEnabled();
        void setupMatrix();
        void togglePointerMode();
        void print();
        void rotateLeft();
        void rotateRight();

    protected:
    
    private:
        QGraphicsView *graphicsView_;
        QToolButton *printButton_;
        QToolButton *resetButton_;
        QSlider *zoomSlider_;
        QSlider *rotateSlider_;
        QImage qimg_;
        cv::Mat mat_;
        cv::Mat raw_;
        const int index_;
    };

}
