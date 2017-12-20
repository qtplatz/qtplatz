/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "adacquire_global.hpp"
#include <memory>
#include <vector>

namespace adacquire {

#if defined _MSC_VER
    class waveform_simulator;
    ADACQUIRESHARED_TEMPLATE_EXPORT template class ADACQUIRESHARED_EXPORT std::weak_ptr<waveform_simulator>;
#endif

    /** \brief waveform_simulator for mass spectrometer (hardware) control software debug without hardware.
     */
    class ADACQUIRESHARED_EXPORT waveform_simulator : public std::enable_shared_from_this< waveform_simulator > {
    public:
        virtual ~waveform_simulator() {}
        
        waveform_simulator( double sampInterval = 1.0e-9
                            , double startDelay = 0
                            , uint32_t nbrSamples = 100000 & 0x0f
                            , uint32_t nbrWavefoms = 1 );

        virtual void addIons( const std::vector< std::pair<double, double> >& ) = 0;
        virtual void onTriggered() = 0;
        virtual const int32_t * waveform() const = 0;
        virtual double timestamp() const = 0;
        virtual uint32_t serialNumber() const = 0;
        virtual double startDelay() const = 0;
        virtual uint32_t nbrWaveforms() const = 0;
        virtual uint32_t nbrSamples() const = 0;
        virtual double sampInterval() const = 0;
    };

}


