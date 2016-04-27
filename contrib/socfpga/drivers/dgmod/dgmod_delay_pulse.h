/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#ifndef SOCFPGA_DRIVERS_DGMOD_DELAY_PULSE_H
#define SOCFPGA_DRIVERS_DGMOD_DELAY_PULSE_H

#if defined __cplusplus
# include <cstdint>
#endif

struct dgmod_delay_pulse {
    uint32_t delay_;
    uint32_t width_;
};

enum { number_of_channels = 6 };

struct dgmod_protocol {
    uint32_t replicates_;
    struct dgmod_delay_pulse delay_pulses_[ number_of_channels ];
};

struct dgmod_protocol_sequence {
    uint32_t interval_; // a.k.a. 'To'
    uint32_t size_;
    struct dgmod_protocol protocols_[ 4 ];
};

#endif
