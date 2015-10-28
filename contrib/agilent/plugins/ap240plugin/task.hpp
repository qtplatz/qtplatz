/**************************************************************************
** Copyright (C) 2014-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this 
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

#include <cstdint>
#include <future>
#include <vector>

class QRect;

namespace adicontroller {
    namespace SignalObserver { class Observer; }
    namespace Instrument { class Session; }
}

namespace adcontrols { class SampleRun; namespace ControlMethod { class Method; } }

namespace ap240 {

    class task {
        ~task();
        task();
        task( const task& ) = delete;
        const task& operator = ( const task& ) = delete;
    public:
        static task * instance();
        bool initialize();
        bool finalize();

        void onDataChanged( adicontroller::SignalObserver::Observer *, uint32_t pos );
        void instInitialize( adicontroller::Instrument::Session * session );
        void post( std::vector< std::future<bool> >& futures );
        void prepare_next_sample( adicontroller::SignalObserver::Observer *
                                  , std::shared_ptr< adcontrols::SampleRun >, const adcontrols::ControlMethod::Method& );

        void clear_histogram();
        void setCellSelectionEnabled( bool );
        void setCellSelection( const QRect& );
        void setHistogramWindowEnabled( bool );
        void setHistogramWindow( double delay, double window );
        void setDeviceDelay( double delay );
        void setHistogramClearCycleEnabled( bool );
        void setHistogramClearCycle( uint32_t );

    private:
        class impl;
        impl * impl_;
    };
}

