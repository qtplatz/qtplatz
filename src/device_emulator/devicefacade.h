// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <QObject>
#include <boost/variant.hpp>
#include <adportable/protocollifecycle.h>

template<class T, class X> class ACE_Singleton;
class ACE_Recursive_Thread_Mutex;
class ACE_Message_Block;
class ACE_INET_Addr;
class DeviceFacadeImpl;
class RoleAverager;
class RoleAnalyzer;
class RoleESI;

typedef boost::variant<RoleAverager, RoleAnalyzer, RoleESI> device_facade_type;

class DeviceFacade : public QObject {
    Q_OBJECT
    ~DeviceFacade();
    DeviceFacade();
    friend ACE_Singleton<DeviceFacade, ACE_Recursive_Thread_Mutex>;
public:
    // attach/detach device, return true if supported otherwise false
    bool attach_device( device_facade_type& );
    bool detach_device( device_facade_type& );

    bool initialize();

    typedef adportable::protocol::LifeCycleFrame LifeCycleFrame;
    typedef adportable::protocol::LifeCycleData LifeCycleData;

    bool handle_dgram( const LifeCycleFrame&, const LifeCycleData&, LifeCycleData& );
    bool lifeCycleUpdate( adportable::protocol::LifeCycleCommand );

    const adportable::protocol::LifeCycle& lifeCycle() const { return lifeCycle_; }
    const ACE_INET_Addr& get_remote_addr() const;

protected:
    bool notify_dgram( ACE_Message_Block * );

private:
    DeviceFacadeImpl * pImpl_;
    adportable::protocol::LifeCycle lifeCycle_;

signals:
    void signal_device_attached( std::string device );
    void signal_device_detached( std::string device );
    void signal_dgram( ACE_Message_Block * );
    void signal_debug( QString msg );
};

typedef ACE_Singleton<DeviceFacade, ACE_Recursive_Thread_Mutex> device_facade;
