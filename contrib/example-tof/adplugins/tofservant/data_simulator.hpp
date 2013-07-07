/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef DATA_SIMULATOR_HPP
#define DATA_SIMULATOR_HPP

#include <boost/date_time/posix_time/posix_time.hpp>
#include <array>

namespace tofservant {

class data_simulator {
public:
    data_simulator();

    enum { ndata = 65536 };

    void peakwidth( double );
    double peakwidth() const;
    void generate_spectrum( size_t nAverage = 1 );
    const std::array< int32_t, ndata >& intensities() const { return intensities_; }
    const std::array< double, 256 >& trace() const { return trace_; }

    static double index_to_mass( size_t idx );
    static size_t mass_to_index( double mass );

private:
    static boost::posix_time::ptime uptime_;
    double peakwidth_;
    std::array< int32_t, ndata > intensities_;
    std::array< double, ndata > rawSpectrum_;
    std::size_t ndata_;
    std::array< double, 256 > trace_;
};

}

#endif // DATA_SIMULATOR_HPP
