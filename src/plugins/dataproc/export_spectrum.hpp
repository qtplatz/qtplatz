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

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/segment_wrapper.hpp>

namespace dataproc {

	struct export_spectrum {
		static bool write( std::ostream& o, const adcontrols::MassSpectrum& _ms ) {
			adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( _ms );
			for ( auto& ms : segments ) { // size_t n = 0; n < segments.size(); ++n ) {
				for ( size_t n = 0; n < ms.size(); ++n ) {
					o << std::scientific << std::setprecision( 15 ) << ms.time( n ) << ",\t"
                      << std::fixed << std::setprecision( 15 ) << ms.mass( n ) << ",\t"
                      << std::scientific << std::setprecision(15) << ms.intensity( n );
                    if ( ms.isCentroid() ) {
                        o << ",\t" << ms.color( n );
                    }
                    o << std::endl;
				}
			}
			return true;
		}
	};
}
