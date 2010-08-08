// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#if defined WIN32
# define USE_MSXML
# define USE_QTXML
#endif

#if defined USE_MSXML
# include "msxml.h"
#endif

#if defined USE_QTXML
# include "qtxml.h"
#endif

#if defined USE_XERCES
# include "xerces.h"
#endif

namespace xmlwrapper {
   // abstruct class to be added
}

