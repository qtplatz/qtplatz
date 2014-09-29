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

#ifndef TIMINGCHART_HPP
#define TIMINGCHART_HPP

#include <QWidget>
#include "plot.hpp"
#include "adplot_global.hpp"

namespace adplot {

    class ADPLOTSHARED_EXPORT TimingChart : public plot {
        Q_OBJECT

        TimingChart( const TimingChart& ) = delete;
        const TimingChart& operator = ( const TimingChart& ) = delete;
    public:
        ~TimingChart();
        explicit TimingChart( QWidget *parent = 0 );

        struct ADPLOTSHARED_EXPORT Pulse {
            Pulse( double delay = 0 , double duration = 0, const QString& name = QString() );
            Pulse( const Pulse& );
            double delay( bool microseconds = true ) const;
            void delay( double, bool microseconds = true );
            double duration( bool microseconds = true ) const;
            void duration( double, bool microseconds = true );
            const QString& name() const;
            void name( const QString& );
            int uniqId() const;
        private:
            friend class TimingChart;
            int uniqId_;
            double delay_;
            double duration_;
            QString name_;
        };

        TimingChart& operator << ( const Pulse& );

    signals:

    public slots :

    private:
        class impl;
        impl * impl_;
        friend class TimingChartPicker;

        void move( const QPoint& );
        void select( const QPoint& );
        void release();
        void showCursor( bool enable );
    };

}

#endif // TIMINGCHART_HPP
