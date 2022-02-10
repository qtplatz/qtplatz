// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#ifndef TXTSPECTRUM_H
#define TXTSPECTRUM_H

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <memory>
#include <vector>

namespace adtextfile {

    class Dialog;

    class TXTSpectrum {
    public:
        TXTSpectrum();
        bool load( const std::wstring&, const Dialog& );

        double minValue_;
        double maxValue_;

        std::shared_ptr< adcontrols::MassSpectrum > compiled_;
        std::vector< std::shared_ptr< adcontrols::MassSpectrum > > spectra_;

    private:
        bool analyze_segments( std::vector< adcontrols::SamplingInfo >&
                               , const std::vector<double>&
                               , const adcontrols::MassSpectrum* compiled  );
        bool validate_segments( const std::vector< adcontrols::SamplingInfo >&, const std::vector<double>& );
        size_t create_spectrum( adcontrols::MassSpectrum&
                                , size_t idx
                                , const adcontrols::SamplingInfo&
                                , const std::vector<double>&
                                , const std::vector<double>&
                                , const std::vector<double>&
                                , size_t fcn );

        int find_mode( size_t idx ) const;
        bool csv_read_tokenizer();
    };

}

#endif // TXTSPECTRUM_H
