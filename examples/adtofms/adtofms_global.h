// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ADTOFMS_GLOBAL_H
#define ADTOFMS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ADTOFMS_LIBRARY)
#  define ADTOFMSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define ADTOFMSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ADTOFMS_GLOBAL_H
