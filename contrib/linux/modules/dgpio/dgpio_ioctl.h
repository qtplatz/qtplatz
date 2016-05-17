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

#ifndef SOCFPGA_DRIVERS_DGPIO_IOCTL_H
#define SOCFPGA_DRIVERS_DGPIO_IOCTL_H

#include <linux/ioctl.h>

#define DGPIO_GET_VERSION       _IOR ( 'r',  0, int )
#define DGPIO_GET_DATA          _IOR ( 'r',  1, int ) // read data byte
#define DGPIO_SET_DATA          _IOR ( 'r',  2, int ) // set data byte

#endif
