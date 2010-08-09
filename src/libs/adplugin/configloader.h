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

namespace adplugin {

	namespace internal {

		class ConfigLoader {
		public:
			ConfigLoader(void);
			~ConfigLoader(void);
			static bool loadConfiguration( adportable::Configuration&, const std::wstring& file, const std::wstring& query );
		};

	}
}
