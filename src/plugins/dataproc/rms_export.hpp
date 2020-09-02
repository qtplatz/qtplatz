/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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

#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <utility>
#include <tuple>

namespace adcontrols { class MassSpectrum; }

namespace dataproc {

    class rms_export {
    public:
        static void text_export( const boost::filesystem::path&, const std::pair<double,double>&, bool axisIsTime );
        static void sqlite_export(  const boost::filesystem::path&, const std::pair<double,double>&, bool axisIsTime );
        static boost::optional<
            std::tuple< std::pair<double, double> // t0,t1
                        , size_t // N
                        , double // rms
                        , double // min time
                        , double // min value
                        , double // max time
                        , double // max value
                        >
            > compute_rms( const adcontrols::MassSpectrum&, const std::pair< double, double >&, bool rangeIsTime );
    };

}
