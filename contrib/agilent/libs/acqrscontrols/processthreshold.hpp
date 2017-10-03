/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "acqrscontrols_global.hpp"
#include "constants.hpp"
#include "ap240/waveform.hpp"
#include "u5303a/waveform.hpp"
#include "threshold_result.hpp"
#include <mutex>
#include <type_traits>

namespace adcontrols {
    class CountingMethod;
    class threshold_method;
}

namespace acqrscontrols {

    class tdcbase;

    template< unsigned int algo>
    class processThreshold_ {
    public:

        processThreshold_() {}

        template< typename waveform_type, size_t nchannels >
        std::array< std::shared_ptr< threshold_result_< waveform_type > >, nchannels >
        operator()( std::array< std::shared_ptr< const waveform_type >, nchannels > waveforms, tdcbase& t ) const;
    };

    /////////////////
    template<>
    template<>
    std::array< std::shared_ptr< threshold_result_< ap240::waveform > >, ap240::nchannels >
    processThreshold_<3>::operator()< ap240::waveform >( std::array< std::shared_ptr< const ap240::waveform >, ap240::nchannels > waveforms, tdcbase& t ) const;

    /////////////////
    template<>
    template<>
    std::array< std::shared_ptr< threshold_result_< u5303a::waveform > >, u5303a::nchannels >
    processThreshold_<3>::operator()< u5303a::waveform >( std::array< std::shared_ptr< const u5303a::waveform >, u5303a::nchannels > waveforms, tdcbase& t ) const;
}
