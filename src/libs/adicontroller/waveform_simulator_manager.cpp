// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "adicontroller_global.hpp"
#include "waveform_simulator_manager.hpp"
#include <functional>

using namespace adicontroller;

waveform_simulator_manager::waveform_simulator_manager()
{
}

waveform_simulator_manager::~waveform_simulator_manager()
{
}

waveform_simulator_manager&
waveform_simulator_manager::instance()
{
    static waveform_simulator_manager __instance__;
    return __instance__;
}

void
waveform_simulator_manager::install_factory( factory_type f )
{
    factory_ = f;
}

std::shared_ptr< adicontroller::waveform_simulator >
waveform_simulator_manager::waveform_simulator( double sampInterval
                                                , double startDelay
                                                , uint32_t nbrSamples
                                                , uint32_t nbrWavefoms ) const
{
    if ( factory_ )
        return factory_( sampInterval, startDelay, nbrSamples, nbrWavefoms );
    return 0;
}
