/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#pragma once
//-----------------

#if defined(_MSC_VER)
#  define DECL_EXPORT __declspec(dllexport)
#  define DECL_IMPORT __declspec(dllimport)
#elif defined __GNUC__
#  define DECL_EXPORT __attribute__((visibility("default")))
#  define DECL_HIDDEN __attribute__((visibility("hidden")))
#  define DECL_IMPORT /* nothing */
#else
#  define DECL_EXPORT /* nothing */
#  define DECL_IMPORT /* nothing */
#endif

#if defined _MSC_VER 
  typedef unsigned int size_t;
#elif defined __APPLE__ 
  typedef unsigned long size_t;
#else
  typedef unsigned int size_t;
#endif
