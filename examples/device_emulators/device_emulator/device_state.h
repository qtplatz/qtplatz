// This is a -*- C++ -*- header.
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

#ifndef DEVICE_STATE_H
#define DEVICE_STATE_H

class device_state {
public:
    ~device_state();
    device_state();
    device_state( const device_state& );

    enum enum_state {
        state_off,
        state_initializing,
        state_diagnostic,
        state_error,
        state_ready,
        state_run,
        state_error_return,
    };

    enum enum_command {
        command_off,
        command_initialize,
        command_do_diagnostic,
        command_begin_transaction, // send voltage1, voltage2..., command_start, command_commit will start hardware
        command_commit,
        command_start,
        command_stop,
    };

    inline enum_state state() const { return state_; }
    void doit( enum_command );
    virtual void activate() {}
    virtual void deactivate() {}
protected:
    enum_state state_;
};

#endif // DEVICE_STATE_H
