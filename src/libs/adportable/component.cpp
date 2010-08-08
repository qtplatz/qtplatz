//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "component.h"

using namespace adportable;

Component::~Component(void)
{
}

Component::Component(void)
{
}

Component::Component( const Component& t )
: name_(t.name_)
, type_(t.type_)
{
}

const std::wstring&
Component::name() const
{
    return name_;
}

void
Component::name( const std::wstring& value )
{
    name_ = value;
}

const std::wstring&
Component::type() const
{
    return type_;
}

void
Component::type( const std::wstring& value )
{
    type_ = value;
}

Component::Interface::~Interface()
{
}

Component::Interface::Interface( const std::wstring& name
                                , const std::wstring& type
                                , const std::wstring& interface )
: name_(name)
, type_(type)
, interface_(interface)
{
}

Component::Interface::Interface( const Interface& t )
: name_(t.name_)
, type_(t.type_)
, interface_(t.interface_)
{
}


const std::wstring
Component::Interface::type() const  // "widget" | "object"
{
    return type_;
}

void
Component::Interface::type( const std::wstring& value )
{
    type_ = value;
}

const std::wstring
Component::Interface::name() const  // "adtofms::imonitor::ui"
{
    return name_;
}

void
Component::Interface::name( const std::wstring& value )
{
    name_ = value;
}

const std::wstring
Component::Interface::interface() const
{
    return interface_;
}

void
Component::Interface::interface( const std::wstring& value )
{
    interface_ = value;
}

