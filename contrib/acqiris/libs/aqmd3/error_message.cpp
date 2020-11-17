/**************************************************************************
** Copyright (C) 2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "error_message.hpp"
#include <AqMD3.h>
#include <boost/format.hpp>

namespace {

    struct messages {
        const char * const msg;
    };

    const messages bffa_errors [] = {
#define AQMD3_ERROR_CANNOT_RECOVER                          (IVIC_ERROR_BASE + 0x0000)
#define AQMD3_ERROR_INSTRUMENT_STATUS                       (IVIC_ERROR_BASE + 0x0001)
#define AQMD3_ERROR_CANNOT_OPEN_FILE                        (IVIC_ERROR_BASE + 0x0002)
#define AQMD3_ERROR_READING_FILE                            (IVIC_ERROR_BASE + 0x0003)
#define AQMD3_ERROR_WRITING_FILE                            (IVIC_ERROR_BASE + 0x0004)
#define AQMD3_ERROR_INVALID_PATHNAME                        (IVIC_ERROR_BASE + 0x000B)
#define AQMD3_ERROR_INVALID_ATTRIBUTE                       (IVIC_ERROR_BASE + 0x000C)
#define AQMD3_ERROR_IVI_ATTR_NOT_WRITABLE                   (IVIC_ERROR_BASE + 0x000D)
#define AQMD3_ERROR_IVI_ATTR_NOT_READABLE                   (IVIC_ERROR_BASE + 0x000E)
#define AQMD3_ERROR_INVALID_VALUE                           (IVIC_ERROR_BASE + 0x0010)
#define AQMD3_ERROR_FUNCTION_NOT_SUPPORTED                  (IVIC_ERROR_BASE + 0x0011)
#define AQMD3_ERROR_ATTRIBUTE_NOT_SUPPORTED                 (IVIC_ERROR_BASE + 0x0012)
#define AQMD3_ERROR_VALUE_NOT_SUPPORTED                     (IVIC_ERROR_BASE + 0x0013)
#define AQMD3_ERROR_TYPES_DO_NOT_MATCH                      (IVIC_ERROR_BASE + 0x0015)
#define AQMD3_ERROR_NOT_INITIALIZED                         (IVIC_ERROR_BASE + 0x001D)
#define AQMD3_ERROR_UNKNOWN_CHANNEL_NAME                    (IVIC_ERROR_BASE + 0x0020)
#define AQMD3_ERROR_TOO_MANY_OPEN_FILES                     (IVIC_ERROR_BASE + 0x0023)
#define AQMD3_ERROR_CHANNEL_NAME_REQUIRED                   (IVIC_ERROR_BASE + 0x0044)
#define AQMD3_ERROR_MISSING_OPTION_NAME                     (IVIC_ERROR_BASE + 0x0049)
#define AQMD3_ERROR_MISSING_OPTION_VALUE                    (IVIC_ERROR_BASE + 0x004A)
#define AQMD3_ERROR_BAD_OPTION_NAME                         (IVIC_ERROR_BASE + 0x004B)
#define AQMD3_ERROR_BAD_OPTION_VALUE                        (IVIC_ERROR_BASE + 0x004C)
#define AQMD3_ERROR_OUT_OF_MEMORY                           (IVIC_ERROR_BASE + 0x0056)
#define AQMD3_ERROR_OPERATION_PENDING                       (IVIC_ERROR_BASE + 0x0057)
#define AQMD3_ERROR_NULL_POINTER                            (IVIC_ERROR_BASE + 0x0058)
#define AQMD3_ERROR_UNEXPECTED_RESPONSE                     (IVIC_ERROR_BASE + 0x0059)
#define AQMD3_ERROR_FILE_NOT_FOUND                          (IVIC_ERROR_BASE + 0x005B)
#define AQMD3_ERROR_INVALID_FILE_FORMAT                     (IVIC_ERROR_BASE + 0x005C)
#define AQMD3_ERROR_STATUS_NOT_AVAILABLE                    (IVIC_ERROR_BASE + 0x005D)
#define AQMD3_ERROR_ID_QUERY_FAILED                         (IVIC_ERROR_BASE + 0x005E)
#define AQMD3_ERROR_RESET_FAILED                            (IVIC_ERROR_BASE + 0x005F)
#define AQMD3_ERROR_RESOURCE_UNKNOWN                        (IVIC_ERROR_BASE + 0x0060)
#define AQMD3_ERROR_ALREADY_INITIALIZED                     (IVIC_ERROR_BASE + 0x0061)
#define AQMD3_ERROR_CANNOT_CHANGE_SIMULATION_STATE          (IVIC_ERROR_BASE + 0x0062)
#define AQMD3_ERROR_INVALID_NUMBER_OF_LEVELS_IN_SELECTOR    (IVIC_ERROR_BASE + 0x0063)
#define AQMD3_ERROR_INVALID_RANGE_IN_SELECTOR               (IVIC_ERROR_BASE + 0x0064)
#define AQMD3_ERROR_UNKOWN_NAME_IN_SELECTOR                 (IVIC_ERROR_BASE + 0x0065)
#define AQMD3_ERROR_BADLY_FORMED_SELECTOR                   (IVIC_ERROR_BASE + 0x0066)
#define AQMD3_ERROR_UNKNOWN_PHYSICAL_IDENTIFIER             (IVIC_ERROR_BASE + 0x0067)

        { "Failure cannot recover." }
        , { "Failure cannot recover." }
    };

}


using namespace aqmd3;

std::string
error_message::operator [] ( uint32_t rcode ) const
{
    switch ( rcode ) {
    case AQMD3_ERROR_CANNOT_RECOVER:            return "AQMD3_ERROR_CANNOT_RECOVER";
    case AQMD3_ERROR_INSTRUMENT_STATUS:         return "AQMD3_ERROR_INSTRUMENT_STATUS";
    case AQMD3_ERROR_CANNOT_OPEN_FILE:          return "AQMD3_ERROR_CANNOT_OPEN_FILE";
    case AQMD3_ERROR_READING_FILE:              return "AQMD3_ERROR_READING_FILE";
    case AQMD3_ERROR_WRITING_FILE:              return "AQMD3_ERROR_WRITING_FILE";
    case AQMD3_ERROR_INVALID_PATHNAME:          return "AQMD3_ERROR_INVALID_PATHNAME";
    case AQMD3_ERROR_INVALID_ATTRIBUTE:         return "AQMD3_ERROR_INVALID_ATTRIBUTE";
    case AQMD3_ERROR_IVI_ATTR_NOT_WRITABLE:	    return "AQMD3_ERROR_IVI_ATTR_NOT_WRITABLE";
    case AQMD3_ERROR_IVI_ATTR_NOT_READABLE:	    return "AQMD3_ERROR_IVI_ATTR_NOT_READABLE";
    case AQMD3_ERROR_INVALID_VALUE:             return "AQMD3_ERROR_INVALID_VALUE";
    case AQMD3_ERROR_FUNCTION_NOT_SUPPORTED:	return "AQMD3_ERROR_FUNCTION_NOT_SUPPORTED";
    case AQMD3_ERROR_ATTRIBUTE_NOT_SUPPORTED:	return "AQMD3_ERROR_ATTRIBUTE_NOT_SUPPORTED";
    case AQMD3_ERROR_VALUE_NOT_SUPPORTED:       return "AQMD3_ERROR_VALUE_NOT_SUPPORTED";
    case AQMD3_ERROR_TYPES_DO_NOT_MATCH:        return "AQMD3_ERROR_TYPES_DO_NOT_MATCH";
    case AQMD3_ERROR_NOT_INITIALIZED:           return "AQMD3_ERROR_NOT_INITIALIZED";
    case AQMD3_ERROR_UNKNOWN_CHANNEL_NAME:	    return "AQMD3_ERROR_UNKNOWN_CHANNEL_NAME";
    case AQMD3_ERROR_TOO_MANY_OPEN_FILES:	    return "AQMD3_ERROR_TOO_MANY_OPEN_FILES";
    case AQMD3_ERROR_CHANNEL_NAME_REQUIRED:	    return "AQMD3_ERROR_CHANNEL_NAME_REQUIRED";
    case AQMD3_ERROR_MISSING_OPTION_NAME:	    return "AQMD3_ERROR_MISSING_OPTION_NAME";
    case AQMD3_ERROR_MISSING_OPTION_VALUE:	    return "AQMD3_ERROR_MISSING_OPTION_VALUE";
    case AQMD3_ERROR_BAD_OPTION_NAME:           return "AQMD3_ERROR_BAD_OPTION_NAME";
    case AQMD3_ERROR_BAD_OPTION_VALUE:          return "AQMD3_ERROR_BAD_OPTION_VALUE";
    case AQMD3_ERROR_OUT_OF_MEMORY:             return "AQMD3_ERROR_OUT_OF_MEMORY";
    case AQMD3_ERROR_OPERATION_PENDING:         return "AQMD3_ERROR_OPERATION_PENDING";
    case AQMD3_ERROR_NULL_POINTER:              return "AQMD3_ERROR_NULL_POINTER";
    case AQMD3_ERROR_UNEXPECTED_RESPONSE:	    return "AQMD3_ERROR_UNEXPECTED_RESPONSE";
    case AQMD3_ERROR_FILE_NOT_FOUND:            return "AQMD3_ERROR_FILE_NOT_FOUND";
    case AQMD3_ERROR_INVALID_FILE_FORMAT:	    return "AQMD3_ERROR_INVALID_FILE_FORMAT";
    case AQMD3_ERROR_STATUS_NOT_AVAILABLE:	    return "AQMD3_ERROR_STATUS_NOT_AVAILABLE";
    case AQMD3_ERROR_ID_QUERY_FAILED:           return "AQMD3_ERROR_ID_QUERY_FAILED";
    case AQMD3_ERROR_RESET_FAILED:              return "AQMD3_ERROR_RESET_FAILED";
    case AQMD3_ERROR_RESOURCE_UNKNOWN:          return "AQMD3_ERROR_RESOURCE_UNKNOWN";
    case AQMD3_ERROR_ALREADY_INITIALIZED:	    return "AQMD3_ERROR_ALREADY_INITIALIZED";
    case AQMD3_ERROR_CANNOT_CHANGE_SIMULATION_STATE:	return "AQMD3_ERROR_CANNOT_CHANGE_SIMULATION_STATE";
    case AQMD3_ERROR_INVALID_NUMBER_OF_LEVELS_IN_SELECTOR:	return "AQMD3_ERROR_INVALID_NUMBER_OF_LEVELS_IN_SELECTOR";
    case AQMD3_ERROR_INVALID_RANGE_IN_SELECTOR:	return "AQMD3_ERROR_INVALID_RANGE_IN_SELECTOR";
    case AQMD3_ERROR_UNKOWN_NAME_IN_SELECTOR:	return "AQMD3_ERROR_UNKOWN_NAME_IN_SELECTOR";
    case AQMD3_ERROR_BADLY_FORMED_SELECTOR:	    return "AQMD3_ERROR_BADLY_FORMED_SELECTOR";
    case AQMD3_ERROR_UNKNOWN_PHYSICAL_IDENTIFIER:	return "AQMD3_ERROR_UNKNOWN_PHYSICAL_IDENTIFIER";
    }
    return ( boost::format( "0x%x") % rcode ).str();
}
