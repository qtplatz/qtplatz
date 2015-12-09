// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#ifndef PRAGMA_WARNINGS_HPP
#define PRAGMA_WARNINGS_HPP

#if defined _MSC_VER
# define pragma_diagnostic_push __pragma( warning(push) )
#elif defined __GNUC__
# define pragma_diagnostic_push _Pragma( GCC diagnostic push )
#else
# define pragma_diagnostic_push
#endif

#if defined _MSC_VER
# define pragma_diagnostic_pop __pragma( warning(pop) )
#elif defined __GNUC__
# define pragma_diagnostic_pop _Pragma( GCC diagnostic pop )
#else
# define pragma_diagnostic_pop
#endif

#if defined _MSC_VER
# define pragma_msvc_warning_disable(x)  __pragma( warning(disable: x ))
# define pragma_msvc_warning_default(x)  __pragma( warning(default: x ))
#else
# define pragma_msvc_warning_disable(x)
# define pragma_msvc_warning_default(x)
#endif

#if defined _MSC_VER
# define pragma_msvc_warning_push_disable_4251  __pragma(warning(push)) __pragma( warning(disable:4251) )
# define pragma_msvc_warning_pop  __pragma(warning(pop))
#else
# define pragma_msvc_warning_push_disable_4251
# define pragma_msvc_warning_pop
#endif

#endif
