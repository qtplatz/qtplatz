/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#ifndef ASSIN_MASSES_HPP
#define ASSIN_MASSES_HPP

#include <cstddef>
#include <boost/noncopyable.hpp>

namespace adcontrols {
    class MSAssignedMasses;
    class MassSpectrum;
    class MSReferences;
}

namespace dataproc {

    class assign_masses : boost::noncopyable {
        double tolerance_;
        double threshold_;
        
    public:
        assign_masses( double tolerance, double threshold ) : tolerance_( tolerance ), threshold_( threshold ) {
        }
        bool operator()( adcontrols::MSAssignedMasses&
                         , const adcontrols::MassSpectrum&
                         , const adcontrols::MSReferences&
                         , int mode
                         , int fcn );
    };

}

#endif // ASSIN_MASSES_HPP
