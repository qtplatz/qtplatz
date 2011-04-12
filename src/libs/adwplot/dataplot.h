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

#pragma once

#include <qwt_plot.h>
#include <boost/smart_ptr.hpp>

namespace adwplot {

    class Traces;
    class Trace;
    class Zoomer;
    class PlotPanner;
    class PlotPicker;
    class Annotation;
    class Annotations;

    class Dataplot : public QwtPlot {
        Q_OBJECT
    public:
        explicit Dataplot(QWidget *parent = 0);
        template<class T> T get();

        void setTitle( const std::wstring& );

        Traces traces();
        Annotations annotations();

        void link( Dataplot * );
        void unlink( Dataplot * );

        const std::pair<double, double>& display_range_x() const;
        const std::pair<double, double>& display_range_y() const;
        void display_range_x( const std::pair<double, double>& );
        void display_range_y( const std::pair<double, double>& );
        void display_range_y2( const std::pair<double, double>& );

    private:
        //virtual void OnMouseDown( double x, double y, short button );
        //virtual void OnMouseUp( double x, double y, short Button );
        //virtual void OnMouseMove( double x, double y, short Button );
        //virtual void OnCharacter( long KeyCode );
        //virtual void OnKeyDown( long KeyCode );
        //virtual void OnSetFocus( long hWnd );
        //virtual void OnKillFocus( long hWnd );
        //virtual void OnMouseDblClk( double x, double y, short button );

    protected:
        //virtual void OnRButtonClick( double /* x */, double /* y */ ) { /* do nothing */ }
        //virtual void OnRButtonRange( double /* x1 */, double /* x2 */, double /* y1 */, double /* y2 */ ) { /* do nothing */ }

    signals:
        void signalZoomXY( double x1, double y1, double x2, double y2 );
        void signalZoomXAutoscaleY( double x1, double x2 );

    public slots:
        //virtual void handleZoomXY( double x1, double y1, double x2, double y2 );
        //virtual void handleZoomXAutoscaleY( double x1, double x2 );

    protected slots:

    private:
        std::vector< Trace > traceVec_;
        std::vector< Annotation > annotationVec_;
        boost::scoped_ptr< Zoomer > zoomer1_;  // left bottom
        boost::scoped_ptr< Zoomer > zoomer2_;  // right top
        boost::scoped_ptr< PlotPicker > picker_;
        boost::scoped_ptr< PlotPanner > panner_;
    };

}


