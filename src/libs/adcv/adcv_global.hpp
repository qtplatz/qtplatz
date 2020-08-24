/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#ifndef ADCV_GLOBAL_HPP
#define ADCV_GLOBAL_HPP

#include <QtCore/qglobal.h>

#if defined(ADCV_LIBRARY)
#  define ADCVSHARED_EXPORT Q_DECL_EXPORT
#  define ADCVSHARED_TEMPLATE_EXPORT
#else
#  define ADCVSHARED_EXPORT Q_DECL_IMPORT
#  define ADCVSHARED_TEMPLATE_EXPORT extern
#endif

#endif // ADCV_GLOBAL_HPP
