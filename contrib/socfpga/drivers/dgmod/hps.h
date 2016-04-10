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

#ifndef SOCFPGA_DRIVERS_HPS_H
#define SOCFPGA_DRIVERS_HPS_H

// see: altera/15.1/embedded/ip/altera/hps/altera_hps/hwlib/include/soc_cv_av/socal/hps.h

enum hps_offsets {
    gpio0_addr         = 0xff708000
    , gpio1_addr       = 0xff709000
    , gpio2_addr       = 0xff70a000
    , gpio_swporta_dr  = 0x0   // write output data to output I/O pin
    , gpio_swporta_ddr = 0x04  // configure the direction of I/O pin
    , gpio_ext_porta   = 0x50  // read input data of I/O input pin
    , gpio_user_led    = 358 + 24   // gpio1[24]
    , gpio_user_key    = 358 + 25   // gpio1[25] --> /sys/class/gpio/gpiochip358; 358 + 25
    , gpio_button_pio  = 416   // 0xff210080
    , gpio_dipsw_pio   = 448   // 0xff210080
};

#endif
