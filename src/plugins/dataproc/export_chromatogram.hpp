// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#pragma once

#include <adcontrols/chromatogram.hpp>

namespace dataproc {

	struct export_chromatogram {
		static bool write( std::ostream& o, const adcontrols::Chromatogram& c ) {
            if ( c.tofArray().empty() && c.massArray().empty() ) {
                for ( size_t n = 0; n < c.size(); ++n ) {
                    o << std::scientific << std::setprecision( 15 ) << c.time( n )
                      << "\t"
                      << std::fixed << std::setprecision( 13 ) << c.intensity( n ) << std::endl;
                }
            } else {
                for ( size_t n = 0; n < c.size(); ++n ) {
                    o << std::scientific << std::setprecision( 15 ) << c.time( n )
                      << "\t"
                      << std::fixed << std::setprecision( 13 ) << c.intensity( n )
                      << "\t"
                      << std::scientific << std::setprecision( 15 ) << c.tof( n )
                      << "\t"
                      << std::fixed << std::setprecision( 8 ) << c.mass( n )
                      << std::endl;
                }
            }
            return true;
		}
	};
}
