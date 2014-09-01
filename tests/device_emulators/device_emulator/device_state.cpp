// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "device_state.h"

device_state::~device_state()
{
}

device_state::device_state() : state_( state_off )
{
}

device_state::device_state( const device_state& t ) : state_(t.state_)
{
}

template<enum device_state::enum_command> struct device_command { };

template<> struct device_command<device_state::command_off> {
    static bool doit( device_state::enum_state& state ) {
        switch( state ) {
            case device_state::state_off: 
                state = device_state::state_off;
                return true;
            case device_state::state_initializing: 
            case device_state::state_diagnostic:
            case device_state::state_error:
                break;
            case device_state::state_ready:
                state = device_state::state_off;
                return true;
            case device_state::state_run:
                break;
        }
        return false;
    }
};

template<> struct device_command<device_state::command_initialize> {
    static bool doit( device_state::enum_state& state ) {
        switch( state ) {
            case device_state::state_off: 
                state = device_state::state_off;
                return true;
            case device_state::state_initializing:
            case device_state::state_diagnostic:
            case device_state::state_error:
            case device_state::state_ready:
            case device_state::state_run:
                break;
        }
        return false;
    }
};

template<> struct device_command<device_state::command_do_diagnostic> {
    static bool doit( device_state::enum_state& state ) {
        switch( state ) {
            case device_state::state_off: 
                state = device_state::state_off;
                return true;
            case device_state::state_initializing:
                break;
            case device_state::state_diagnostic:
                break;
            case device_state::state_error:
                break;
            case device_state::state_ready:
                state = device_state::state_diagnostic;
                return true;
            case device_state::state_run:
                break;
        }
        return false;

    }
};

template<> struct device_command<device_state::command_begin_transaction> {
    static bool doit( device_state::enum_state& state ) {
        switch( state ) {
            case device_state::state_off: 
                state = device_state::state_off;
                return true;
            case device_state::state_initializing: 
            case device_state::state_diagnostic:
            case device_state::state_error:
                break;
            case device_state::state_ready:
                return true;
            case device_state::state_run:
                break;
        }
        return false;
    }
};

template<> struct device_command<device_state::command_commit> {
    static bool doit( device_state::enum_state& state ) {
        switch( state ) {
            case device_state::state_off:
                state = device_state::state_off;
                return true;
            case device_state::state_initializing:
            case device_state::state_diagnostic:
            case device_state::state_error:
                break;
            case device_state::state_ready:
                state = device_state::state_ready;
                return true;
            case device_state::state_run:
                break;
        }
        return false;
    }
};

template<> struct device_command<device_state::command_start> {
    static bool doit( device_state::enum_state& state ) {
        switch( state ) {
            case device_state::state_off: 
                state = device_state::state_off;
                return true;
            case device_state::state_initializing: 
                break;
            case device_state::state_diagnostic:
                break;
            case device_state::state_error:
                break;
            case device_state::state_ready:
                state = device_state::state_run;
                return true;
            case device_state::state_run:
                break;
        }
        return false;
    }
};

template<> struct device_command<device_state::command_stop> {
    static bool doit( device_state::enum_state& state ) {
        switch( state ) {
            case device_state::state_off: 
                state = device_state::state_off;
                return true;
            case device_state::state_initializing:
                break;
            case device_state::state_diagnostic:
                break;
            case device_state::state_error:
                state = device_state::state_ready;  // error reset
                return true;
            case device_state::state_ready:
                break;
            case device_state::state_run:
                state = device_state::state_ready;
                return true; 
        }
        return false;
    }
};

void
device_state::doit( device_state::enum_command command )
{
    bool res = false;
    switch( command ) {
    case command_off:
        res = device_command<command_off>::doit( state_ );
        break;
    case command_initialize:
        res = device_command<command_initialize>::doit( state_ );
        break;
    case command_do_diagnostic:
        res = device_command<command_do_diagnostic>::doit( state_ );
        break;
    case command_begin_transaction:
        res = device_command<command_begin_transaction>::doit( state_ );
        break;
    case command_commit:
        res = device_command<command_commit>::doit( state_ );
        break;
    case command_start:
        res = device_command<command_start>::doit( state_ );
        break;
    case command_stop:
        res = device_command<command_stop>::doit( state_ );
        break;
    }
}
