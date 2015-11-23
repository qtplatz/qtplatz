// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2015-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015-2016 MS-Cheminformatics LLC
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

#include "mblock.hpp"

namespace adportable {

    class waveform_simulator {

        class impl;
        
    public:
        ~waveform_simulator();
        waveform_simulator();

        waveform_simulator( double delay, size_t actualPoints, double xIncrement );

        void operator()( std::shared_ptr< mblock<int16_t> >&, int numRecords = 1 ) const;

        void operator()( std::shared_ptr< mblock<int32_t> >&, int numRecords = 1 ) const;

        size_t actualPoints() const;
    };
    
}

