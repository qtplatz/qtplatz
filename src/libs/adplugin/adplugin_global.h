// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ADPLUGIN_GLOBAL_H
#define ADPLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ADPLUGIN_LIBRARY)
#  define ADPLUGINSHARED_EXPORT Q_DECL_EXPORT
#else
#  define ADPLUGINSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ADPLUGIN_GLOBAL_H
