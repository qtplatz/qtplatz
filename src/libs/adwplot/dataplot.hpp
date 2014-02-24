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

#include <qwt_plot.h>
#include <adwplot/zoomer.hpp>
#include <boost/noncopyable.hpp>
#include <memory>

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
        void setTitle( const std::string& );
        void setFooter( const std::wstring& );
        void setFooter( const std::string& );
    
        void link( Dataplot * );
        void unlink();

        QRectF zoomRect() const;
        inline Zoomer& zoomer() { return *zoomer1_; } // left bottom
    
    private:
        typedef std::vector<Dataplot *> plotlink;
        std::shared_ptr< plotlink > plotlink_;
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
        std::unique_ptr< Zoomer > zoomer1_;  // left bottom axix
        std::unique_ptr< Picker > picker_;   // (right mouse button)
        std::unique_ptr< Panner > panner_;
    };
  
}


