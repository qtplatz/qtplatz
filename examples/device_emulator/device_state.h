// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

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
