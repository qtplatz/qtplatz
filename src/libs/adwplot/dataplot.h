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

namespace adwplot {

    class Traces;
    class Trace;

    class Dataplot : public QwtPlot {
        Q_OBJECT
    public:
        explicit Dataplot(QWidget *parent = 0);
        template<class T> T get();

        Traces traces();

    signals:
/*
        void signalMouseDown( double x, double y, short button );
        void signalMouseUp( double x, double y, short Button );
        void signalMouseMove( double x, double y, short Button );
        void signalCharacter( long KeyCode );
        void signalKeyDown( long KeyCode );
        void signalSetFocus( long hWnd );
        void signalKillFocus( long hWnd );
        void signalMouseDblClk(double x, double y, short button );
*/	  
    public slots:
    protected slots:
/*
        virtual void OnMouseDown( double, double, short ) {}
        virtual void OnMouseUp( double, double, short ) {}
        virtual void OnMouseMove( double, double, short ) {}
        virtual void OnCharacter( long ) {}
        virtual void OnKeyDown( long ) {}
        virtual void OnSetFocus( long ) {}
        virtual void OnKillFocus( long ) {}
        virtual void OnMouseDblClk(double, double, short ) {}
*/
    private:
        std::vector< Trace > traceVec_;
    };

}


