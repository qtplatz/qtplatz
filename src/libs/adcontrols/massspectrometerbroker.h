// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include "visitor.h"
#include <string>

namespace adcontrols {

    class MassSpectrometer;

    class ADCONTROLSSHARED_EXPORT MassSpectrometerBroker : public Visitor {
	protected:
		MassSpectrometerBroker(void);
		~MassSpectrometerBroker(void);
	public:
        // virtual void visit( adcontrols::MassSpectrometer& )

		typedef MassSpectrometer * (*factory_type)(void);

        static bool register_library( const std::wstring& sharedlib );
		static bool register_factory( factory_type, const std::wstring& name );
		static factory_type find( const std::wstring& name );
	};

}
