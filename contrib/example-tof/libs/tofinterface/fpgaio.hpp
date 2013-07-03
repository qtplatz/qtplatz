/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#ifndef FPGAIO_HPP
#define FPGAIO_HPP

namespace tofinterface {

    template<size_t> struct dma;

    enum { ndword_size_out   = 65536 + 1024 }; // 65536 dword + 512
    enum { ndword_size_in    = 65536 };        // mass command has 32768 words x 3

    typedef dma< ndword_size_out >   fpga_out;
    typedef dma< ndword_size_in >    fpga_in;

    namespace fpga {
        namespace constants {
            const char * const object_name_out   = "fpgaOutObject";
            const char * const object_name_in    = "fpgaInObject";
            // const char * const object_name_RF    = "fpgaOutRF";
        }
    }

}

#endif // FPGAIO_HPP
