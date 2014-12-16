/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <compiler/decl_export.h>
#include <cstdint>

#if defined(EVENTBROKER_LIBRARY)
#  define EVENTBROKER_EXPORT DECL_EXPORT
#else
#  define EVENTBROKER_EXPORT DECL_IMPORT
#endif

extern "C" {

    typedef void (*event_handler)(uint32_t, void*);

    EVENTBROKER_EXPORT uint32_t eventbroker_count_if();
    EVENTBROKER_EXPORT const char * eventbroker_if( uint32_t idx );
    EVENTBROKER_EXPORT const char * eventbroker_ipaddr( uint32_t idx );
    EVENTBROKER_EXPORT const bool eventbroker_regiser_handler( event_handler );
    EVENTBROKER_EXPORT const bool eventbroker_unregiser_handler( event_handler );

    EVENTBROKER_EXPORT void eventbroker_out( uint32_t );

}

