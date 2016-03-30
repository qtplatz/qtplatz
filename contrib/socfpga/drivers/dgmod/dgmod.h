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

#ifndef SOCFPGA_DRIVERS_DGMOD_H
#define SOCFPGA_DRIVERS_DGMOD_H

#include <linux/version.h>
#include <linux/cdev.h>

#define MODNAME     "dgmod"
#define CDEV_NAME   MODNAME
#define MOD_VERSION "1.0"

//#define IRQ_NUM      104
#define IRQ_NUM      72

enum {
    map_base_addr = 0xff200000
    , map_size = 0x20000
    , pio_base = 0x10040
    , addr_machine_state = 0x10100 / sizeof(uint32_t)
    , addr_submit        = 0x10120 / sizeof(uint32_t)
    , addr_interval      = 0x10180 / sizeof(uint32_t)
    , addr_revision      = 0x101a0 / sizeof(uint32_t)
};

enum fsm_state {
    fsm_stop     = 0x0000
    , fsm_start  = 0x0001
    , fsm_update = 0x0001
};

struct pulse_addr {  uint32_t delay; uint32_t width; };

#endif
