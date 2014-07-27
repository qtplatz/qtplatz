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

#ifndef MSFINDER_HPP
#define MSFINDER_HPP

#include "adcontrols_global.h"

namespace adcontrols {

    class MassSpectrum;

    enum idFindAlgorithm : int {
        idFindLargest
        , idFindClosest
    };
        
    enum idToleranceMethod : int {
        idToleranceDaltons
        , idTolerancePpm
    };

    class ADCONTROLSSHARED_EXPORT MSFinder  {
    public:

        ~MSFinder();
        MSFinder( double width = 0.005, idFindAlgorithm a = idFindLargest, idToleranceMethod w = idToleranceDaltons );
        MSFinder( const MSFinder& );

        static const size_t npos = size_t( -1 ); // no peak has found

        size_t operator()( const MassSpectrum&, double mass );
        
        double width() const { return width_; }
        idToleranceMethod toleranceMethod() const { return toleranceMethod_; }
        idFindAlgorithm findAlgorithm() const { return findAlgorithm_; }

    private:
        double width_;
        idToleranceMethod toleranceMethod_;
        idFindAlgorithm findAlgorithm_;
    };

}

#endif // MSFINDER_HPP
