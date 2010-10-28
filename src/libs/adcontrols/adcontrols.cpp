//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#define BOOST_LIB_NAME boost_serialization
#include <boost/config/auto_link.hpp>

#include "ace/Init_ACE.h"

#if defined ACE_WIN32
#  if defined _DEBUG
#     pragma comment(lib, "ACEd.lib")
#     pragma comment(lib, "adportabled.lib")
#  else
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "adportable.lib")
#  endif
#endif
