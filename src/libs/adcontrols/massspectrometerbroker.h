// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <string>

namespace adcontrols {

    class MassSpectrometer;

	class ADCONTROLSSHARED_EXPORT MassSpectrometerBroker {
	protected:
		MassSpectrometerBroker(void);
		~MassSpectrometerBroker(void);
	public:
		typedef MassSpectrometer * (*factory_type)(void);

		static bool register_factory( factory_type, const std::wstring& name );
		static factory_type find( const std::wstring& name );
	};

}
