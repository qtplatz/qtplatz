// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#pragma once

#include "adplot_global.hpp"
#include <qwt_plot.h>
#include <QString>

class QwtPlotPanner;

namespace adplot {

    class Traces;
    class Trace;
    class Zoomer;
    class Picker;
    class Panner;

    class ADPLOTSHARED_EXPORT plot : public QwtPlot {
        Q_OBJECT
    public:
        explicit plot(QWidget *parent = 0);
        ~plot();

		void setTitle( const QString& );
		void setFooter( const QString& );

        void setTitle( const std::wstring& text ) { setTitle( QString::fromStdWString( text ) ); }
        void setTitle( const std::string& text ) { setTitle( QString::fromStdString( text ) ); }

        void setFooter( const std::wstring& text )  { setFooter( QString::fromStdWString( text ) ); }
        void setFooter( const std::string& text )  { setFooter( QString::fromStdString( text ) ); }
    
        void link( plot * );
        void unlink();

        QRectF zoomRect() const;

        virtual void setVectorCompression( int );
        int vectorCompression() const;

        Zoomer * zoomer( int idx = 0 ) const;
        Picker * picker() const;
        Panner * panner() const;

        void setAxisScale( int axisId, double min, double max, double stepSize = 0 );

        virtual void yZoom( double xmin, double xmax );

        static void copyToClipboard( plot * );
        static void copyImageToFile( plot *, const QString& file, const QString& format = "svg", bool compress = false, int dpi = 300 );
    
    private:
        class impl;
        impl * impl_;

    protected:
        virtual void zoom( const QRectF& );
    
    signals:
      
    protected slots:
        virtual void onZoomed( const QRectF& );
        virtual void onPanned( int dx, int dy );
    
    };
  
}


