/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <boost/variant.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace acqrscontrols {
    namespace u5303a { class threshold_result; class waveform; }
    namespace ap240 { class waveform; }
    template< typename T > class threshold_result_;
    typedef threshold_result_< ap240::waveform > ap240_threshold_result;
    enum class Digitizer : unsigned int;
};

namespace adcontrols { class MassSpectrum; }

namespace accutof { namespace acquire {

        class ResultWriter {
        public:
            ResultWriter();
            ~ResultWriter();

            ResultWriter& operator << ( std::shared_ptr< const acqrscontrols::u5303a::threshold_result >&& );
            ResultWriter& operator << ( std::shared_ptr< const acqrscontrols::u5303a::waveform >&& );
            ResultWriter& operator << ( std::shared_ptr< const acqrscontrols::ap240_threshold_result >&& );

            void commitData();
            void fsmStop();

        private:
            template< typename T > void commitData_( std::vector< std::shared_ptr< const T > >& );

            std::mutex mutex_;
            std::vector< std::shared_ptr< const acqrscontrols::u5303a::threshold_result > > cache0_;
            std::vector< std::shared_ptr< const acqrscontrols::ap240_threshold_result > > cache2_;

            std::string time_datafile_;
            bool acquisition_active_;
        };

    }
}
