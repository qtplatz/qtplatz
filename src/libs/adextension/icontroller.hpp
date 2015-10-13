/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include <QObject>
#include "adextension_global.hpp"
#include <functional>
#include <memory>

namespace adcontrols {
    namespace ControlMethod { class Method; }
}

namespace adicontroller {
    namespace Instrument { class Session; }
    namespace SignalObserver { class Observer; class ObserverEvents; }
}

namespace adextension {
    
    class ADEXTENSIONSHARED_EXPORT iController : public QObject
                                               , public std::enable_shared_from_this< iController > {
        Q_OBJECT
    public:
        explicit iController(QObject *parent = 0);
        std::shared_ptr< iController > pThis() { return shared_from_this(); }
        std::shared_ptr< const iController > pThis() const { return shared_from_this(); }        

        virtual bool connect() = 0;
        virtual bool wait_for_connection_ready() = 0;

        // for backword compat; use getInstrumentSession() instead
        virtual bool preparing_for_run( adcontrols::ControlMethod::Method& ) { return false; }

        /*
         * api below this line was defined for pure c++ instrument controller interface -- ported from CORBA api
         */
        virtual adicontroller::Instrument::Session * getInstrumentSession() = 0; // can be nullptr

        /* module_name identify the instrument/peripheral model name
         * which match up with the name on control method item filed
         */
        virtual QString module_name() const = 0;

        /* module_number identify instrument where same instruments are configured
         * such as two UV-ditectors, multiple 6-way valves etc.
         */
        virtual int module_number() const = 0;

        virtual void dataChangedHandler( std::function< void( adicontroller::SignalObserver::Observer *, unsigned int pos ) > ) = 0;

        virtual void dataEventHandler( std::function< void( adicontroller::SignalObserver::Observer *, unsigned int events, unsigned int pos ) > ) = 0;

        /* call backs */
        virtual void invokeDataChanged( adicontroller::SignalObserver::Observer * o, unsigned int pos ) { emit dataChanged( o, pos ); }
        
        virtual void invokeDataEvent( adicontroller::SignalObserver::Observer * o, unsigned int events, unsigned int pos ) { emit dataEvent( o, events, pos ); }

    signals:
        void onControlMethodChanged();
        void connected( iController * self );
        void message( iController * self, unsigned int code, unsigned int value );
        void log( iController * self, const QString& );
        void dataChanged( adicontroller::SignalObserver::Observer *, unsigned int pos );
        void dataEvent( adicontroller::SignalObserver::Observer *, unsigned int events, unsigned int pos );
    };

}

