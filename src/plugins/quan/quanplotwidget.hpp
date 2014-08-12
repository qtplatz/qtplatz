/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANPLOTWIDGET_HPP
#define QUANPLOTWIDGET_HPP

#include <QWidget>
#include <memory>

#include <adwplot/dataplot.hpp>

namespace adwplot { class PeakMarker; }

namespace quan {

    class QuanPlotData;

    namespace detail { template<typename T> struct widget_get; };

    class QuanPlotWidget : public QWidget  {
        Q_OBJECT
    public:
        ~QuanPlotWidget();
        QuanPlotWidget( QWidget * parent = 0 );

        void setData( const QuanPlotData *, size_t idx, int fcn, const std::wstring& dataSource );
        adwplot::Dataplot * dataplot() { return dplot_.get(); }
        void dataplot( adwplot::Dataplot * p ) { dplot_.reset( p ); }
    private:
        std::unique_ptr< adwplot::Dataplot > dplot_;
        std::unique_ptr< adwplot::PeakMarker > marker_;
        void handleDataChanged( int id, bool f );
    };

}

#endif // QUANPLOTWIDGET_HPP
