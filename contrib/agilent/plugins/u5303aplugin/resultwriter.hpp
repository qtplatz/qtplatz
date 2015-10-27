/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <boost/format.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace acqrscontrols { namespace u5303a { class threshold_result; } };
namespace adcontrols { class MassSpectrum; }

namespace u5303a {

    class ResultWriter {
    public:
        ResultWriter();
        ~ResultWriter();

        ResultWriter& operator << ( std::shared_ptr< const acqrscontrols::u5303a::threshold_result > );

        void commitData();
        void writeHistogram( size_t, const std::pair< uint64_t, uint64_t>&, std::shared_ptr< adcontrols::MassSpectrum > );
        
    private:
        std::mutex mutex_;
        std::string time_datafile_;
        std::string hist_datafile_;
        std::vector< std::shared_ptr< const acqrscontrols::u5303a::threshold_result > > cache_;
    };

}

