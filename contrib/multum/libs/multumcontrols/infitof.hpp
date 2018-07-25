/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

namespace infitof {

    namespace Constants {

        const double FLIGHT_LENGTH_L1      = 0.06626 + 0.087863113; // 0.08806; // ion pulse point to injection sector exit (1 -- 2)
        const double FLIGHT_LENGTH_L2      = 0.30512; // injection sector exit to ejection sector entry (2 -- 3)
        const double FLIGHT_LENGTH_L3      = 0.32766; // injection sector exit to 4 (2--4)
        const double FLIGHT_LENGTH_LG      = 0.61973; // injection sector exit to ion gate (2--5)
        const double FLIGHT_LENGTH_L4      = 0.64023; // injection sector exit to ion gate (2--6)
        const double FLIGHT_LENGTH_LT      = 0.66273; // length of figure-eight orbit
        const double FLIGHT_LENGTH_EXIT    = 0.06626; 

        // const char * const httpd_dg = "dg-httpd";
        // const char * const httpd_hv = "hv-httpd";
    };
    
};
