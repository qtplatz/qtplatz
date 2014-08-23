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

#ifndef QUANCALIBRATION_HPP
#define QUANCALIBRATION_HPP

#include "adcontrols_global.h"
#include "idaudit.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <compiler/disable_dll_interface.h>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT QuanCalibration  {
    public:
        ~QuanCalibration();
        QuanCalibration();
        QuanCalibration( const QuanCalibration& );

        enum WEIGHTING {
            WEIGHTING_NONE,
            WEIGHTING_INV_C,
            WEIGHTING_INV_C2,
            WEIGHTING_INV_C3,
        };
        void formula( const char * );
        const char * formula() const;

        const boost::uuids::uuid& uuid_cmpd_table() const;
        void uuid_cmpd_table( const boost::uuids::uuid& );

        const boost::uuids::uuid& uuid_cmpd() const;
        void uuid_cmpd( const boost::uuids::uuid& );

        void operator << (const std::pair<double, double>& amount_intensity_pair);

        size_t size() const;
        const double * x() const;
        const double * y() const;
        double min_x() const;
        double max_x() const;

        bool fit( int nTerm, bool forceOrigin = false, WEIGHTING wType = WEIGHTING_NONE );
        double chisqr() const;
        double estimate_y( double x ) const;
        const double * coefficients() const;
        size_t nTerms() const;
        
    private:
        boost::uuids::uuid idCompound_;
        boost::uuids::uuid idTable_;
        std::string formula_;            // for convenience, equal to QuanCompund.formula
        std::vector< double > x_;        // intensity (x-y exchange due to this is calibrating amount(y) from intensity(x)
        std::vector< double > y_;        // amount
        double chisqr_;
        std::vector< double > coefficients_;
    };

}

#endif // QUANCALIBRATION_HPP
