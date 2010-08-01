// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef ADBROKER_GLOBAL_H
#define ADBROKER_GLOBAL_H

#if defined(ADBROKER_LIBRARY)
#  define ADBROKERSHARED_EXPORT __declspec(dllexport)
#else
#  define ADBROKERSHARED_EXPORT __declspec(dllimport)
#endif

#endif // ADBROKER_GLOBAL_H
