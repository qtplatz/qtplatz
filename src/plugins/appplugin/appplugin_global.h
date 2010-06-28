// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef APPPLUGIN_GLOBAL_H
#define APPPLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(APPPLUGIN_LIBRARY)
#  define APPPLUGIN_EXPORT Q_DECL_EXPORT
#else
#  define APPPLUGIN_EXPORT Q_DECL_IMPORT
#endif

#endif // APPPLUGIN_GLOBAL_H
