// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "device_state.h"
#include <boost/smart_ptr.hpp>

namespace TOFInstrument {

	struct ADCConfiguration;
	struct AnalyzerDeviceData;

}

namespace device_emulator {

  class device_hvcontroller : public device_state {
  public:
	  device_hvcontroller( void );
	  ~device_hvcontroller( void );
	  device_hvcontroller( const device_hvcontroller& );
	  bool operator == ( const device_hvcontroller& ) const;
	  const char * deviceType() const { return "analyzer"; }
  private:
	  boost::shared_ptr< TOFInstrument::ADCConfiguration > adConfig_;
	  boost::shared_ptr< TOFInstrument::AnalyzerDeviceData > data_;
  };

}

