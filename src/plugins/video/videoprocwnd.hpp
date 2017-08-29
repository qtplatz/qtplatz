/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#pragma once

#include <adcontrols/datareader.hpp>
#include <QWidget>
#include <memory>

class QGridLayout;
class QEvent;
class QPainter;
class QPrinter;

namespace portfolio { class Folium; }
namespace adcontrols { class MappedImage; class MappedSpectra; class MassSpectrum; }
namespace adplot { class ChromatogramWidget; }
namespace cv { class Mat; }
namespace advision { class ImageWidget; }

namespace video {

    namespace cv_extension {
        template<typename T, uint> class mat_t;
    };

    class VideoProcWnd : public QWidget {
        Q_OBJECT
    public:
        ~VideoProcWnd();
        explicit VideoProcWnd( QWidget *parent = 0 );

        void setHistogramWindow( double tof, double width );
        void setEnabled( int id, bool enable ); // check/uncheck map rect (0) or tof range(1)

        void print( QPainter&, QPrinter& );

    signals:

    public slots :

    private slots:
        void handleData();
        void handlePlayer( QImage );
        void handleFileChanged( const QString& );

    private:
        std::array< std::unique_ptr< advision::ImageWidget >, 2 > imgWidgets_;
        std::unique_ptr< adplot::ChromatogramWidget > tplot_;
        std::unique_ptr< cv::Mat > average_;
        size_t numAverage_;
    };

}

