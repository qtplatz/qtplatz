// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>

namespace adportable {
	class Configuration;
	class Component;
}

namespace adportable {

    class ConfigLoader {
    public:
        ConfigLoader(void);
        ~ConfigLoader(void);
        static bool loadConfigFile( adportable::Configuration&, const std::wstring& file, const std::wstring& query );
        static bool loadConfigXML( adportable::Configuration&, const std::wstring& xml, const std::wstring& query );
    };

}
