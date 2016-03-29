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

#define DGMOD_MAJOR    MISC_MAJOR
#define DGMOD_MINOR    153

#define IRQ_NUM        72

struct dgmod_device {
    int dev_num;
    dev_t dev;
    wait_queue_head_t wait_proc;
};

struct dgmod_driver {
    struct dgmod_device **devices;
    dev_t dev;
    struct cdev cdev;
};

#endif
