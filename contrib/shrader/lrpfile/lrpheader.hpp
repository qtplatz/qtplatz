/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include <array>
#include <cstdint>
#include <string>

namespace shrader {

    class lrpheader {
        enum { data_size = 256 };
    public:
        ~lrpheader();
        lrpheader( std::istream& in, size_t fsize );
        inline operator bool () const { return loaded_; }

        int32_t flags() const;                  // Long 4 Record type code = 1;
        std::string version() const;            // String 4 AAAD;
        int32_t type() const;                   // Long 4 Data type code;
        std::string analdate() const;           // String 20 Analysis date;
        std::string analtime() const;           // String 8 Time of analysis;
        std::string instrument() const;         // String 40 Instrument description;
        std::string operator_name() const;      // String 40 Operator name;
        std::string calfile() const;            // String 8 Calibration file name;
        std::string library() const;            // String 8 Library name (selected ion monitoring only);
        std::string libcaldate() const;         // String 20 Last calibration date of library;
        int16_t interfacetype() const;          // integer 2 4=Jumbo; 5=MEV;
        int16_t rawdatatype() const;            // integer 2 0=not defined or not raw; 1=drive; 2=time;
        std::string SecondDmension() const;     // String 32;
        size_t AltTicPtr() const;               // Long 4 Pointer to Alternate TIC;
        int32_t nscans() const;                 // Long 4 Number of scans in data file;
        size_t setupptr() const;                //  Long 4 Pointer to beginning of instrument set-up;
        size_t calptr() const;                  // Long 4 Pointer to beginning of calibration information;
        size_t simptr() const;                  //  Long 4 Pointer to beginning of SIM set-up;
        int32_t scanptr() const;                // Long 4 Not used - Record of first spectrum (MSData);
        size_t ticptr() const;                  //  Long 4 Pointer to first TIC master block;
        size_t miscptr() const;                 // Long 4 Pointer to beginning of misc. section;
        size_t labelptr() const;                //  Long 4 Pointer to beginning of scan labels;

        std::string data_type_code() const;
        std::string interfacetype_code() const;
        std::string rawdatatype_code() const;

    private:
        std::array< char, data_size > data_;
        bool loaded_;
    };

}

