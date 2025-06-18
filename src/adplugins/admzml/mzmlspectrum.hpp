// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
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

#include "mzmldatumbase.hpp"
#include "binarydataarray.hpp"
#include "scan_protocol.hpp"
#include <boost/json/fwd.hpp>
#include <memory>

namespace adcontrols {
    class MassSpectrum;
}

namespace mzml {

    enum ion_polarity_type : unsigned int;

    class mzMLSpectrum : public mzMLDatumBase {
        class impl;
        std::unique_ptr< impl > impl_;
    public:
        ~mzMLSpectrum();
        mzMLSpectrum();
        mzMLSpectrum( const mzMLSpectrum& t );
        mzMLSpectrum( binaryDataArray prime
                      , binaryDataArray secondi
                      , pugi::xml_node node );

        size_t length() const;
        boost::json::value to_value() const;
        std::pair< const binaryDataArray&, const binaryDataArray& > dataArrays() const;

        const scan_id& scan_id() const;
        double scan_start_time() const;
        std::pair< double, double > scan_range() const; // lower, upper
        std::pair< double, double > base_peak() const;  // mz, intensity
        double precursor_mz() const;
        int ms_level() const;
        bool is_profile() const;
        ion_polarity_type polarity() const;
        double total_ion_current() const;
        void set_protocol_id( int );
        int protocol_id() const;

        static std::shared_ptr< adcontrols::MassSpectrum > toMassSpectrum( const mzMLSpectrum& );
    };
}
