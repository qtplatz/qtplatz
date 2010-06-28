// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#pragma once

#include <QtCore/qglobal.h>

#if defined(ADCONTROLS_LIBRARY)
#  define ADCONTROLSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define ADCONTROLSSHARED_EXPORT Q_DECL_IMPORT
#endif

