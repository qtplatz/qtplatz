/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef SPCHDR_HPP
#define SPCHDR_HPP

#pragma once

#include "spc_h.hpp"
#include "spcfile_global.hpp"
#include <ostream>

namespace galactic {

    class SPCFILESHARED_EXPORT spchdr {
    public:
        spchdr( const SPCHDR * p );
        const SPCHDR * p() const { return p_; }

        uint8_t ftflgs() const;
        // file data type flags
        bool isMultiFileFormat() const;
        bool isRandomZValues() const;
        bool isOrderedZValues() const;
        bool hasAxisLabel() const;
        bool hasUniqueXArray() const;
        bool hasXArray() const;

        uint8_t fversn() const;
        bool isDeprecated() const;
        bool isLittleEndian() const;

        uint8_t fexper() const;
        int8_t  fexp() const;
        uint32_t fnpts() const;
        double ffirst() const;
        double flast() const;
        uint32_t number_of_subfiles() const;
        uint8_t axis_type_x() const;
        uint8_t axis_type_y() const;
        uint8_t axis_type_z() const;
        uint8_t fpost() const;
        uint32_t fdate() const;
        const char * resolution_description() const;
        const char * source_instrument_description() const;
        int16_t num_peak_points() const;
        const char * comment() const;
        const char * axis_label_text() const;
        uint32_t logblock_offset() const;
        uint32_t file_modification_flags() const;
        uint8_t processing_code() const;
        uint8_t calibration_level() const;
        int16_t sub_method_sample_injection_number() const;
        float  ffactor() const;
        const char * fmethod() const;
        float fzinc() const;
        uint32_t fwplanes() const;
        float  fwinc() const;
        uint8_t fwtype() const;
        void dump_spchdr( std::ostream& ) const;

        static const char * string_from_time( uint32_t, std::string& );

    private:
        const SPCHDR * p_;
    };

}

#endif // SPCHDR_HPP
