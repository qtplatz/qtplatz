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
#include <adwplot/zoomer.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/noncopyable.hpp>

class QwtPlotPanner;

namespace adwplot {

    class Traces;
    class Trace;
    class Zoomer;
	class Picker;
	class Panner;

	class Dataplot : public QwtPlot, boost::noncopyable {
        Q_OBJECT
    public:
        explicit Dataplot(QWidget *parent = 0);
		~Dataplot();
        template<class T> T get();

        void setTitle( const std::wstring& );

        void link( Dataplot * );
        void unlink();

    private:
		typedef std::vector<Dataplot *> plotlink;
		boost::shared_ptr< plotlink > plotlink_;
		bool linkedzoom_inprocess_;
		void zoom( const QRectF&, const Dataplot& );
		void panne( int dx, int dy, const Dataplot& );

    protected:
		virtual void zoom( const QRectF& );

    signals:

    protected slots:
		virtual void onZoomed( const QRectF& );
		virtual void onPanned( int dx, int dy );

    protected:
        boost::scoped_ptr< Zoomer > zoomer1_;  // left bottom
        boost::scoped_ptr< Zoomer > zoomer2_;  // right top
        boost::scoped_ptr< Picker > picker_;
        boost::scoped_ptr< Panner > panner_;
    };

}


