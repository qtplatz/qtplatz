/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/mscalibration.hpp>
#include <memory>

namespace accutof { namespace acquire {

        struct mass_assignor {
            const adcontrols::MSCalibration * calib_;
            std::shared_ptr< const adcontrols::ScanLaw > scanLaw_;

            mass_assignor() : calib_( 0 ), scanLaw_( 0 ) {
                auto sp = document::instance()->massSpectrometer();
                calib_ = sp->findCalibration( 0 );
                // scanLaw_ = sp->scanLaw();
            }

            double operator()( double time, int ) const {
                if ( calib_ )
                    return calib_->compute_mass( time );
                else
                    return document::instance()->massSpectrometer()->assignMass( time, 0 );
            }
        };

    }
}
