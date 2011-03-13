// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>

namespace adfs {

    typedef long int32;
    typedef unsigned long uint32;
    typedef signed long long off_t;
    typedef unsigned long long ino_t;
    typedef signed long long daddr_t; // disk address
    typedef unsigned long agnumber_t; // allocation group number

    typedef unsigned long long time_t;

    typedef unsigned long inode_addr;

    enum mode_t {
        disk_create
        , disk_readwrite
        , disk_readonly
    };

    enum Constants {
        MAX_NAME_LEN = 256
        , ADFS_MAGIC1 = 0x77ddE63D // 2011031101
        , ADFS_MAGIC2 = 0x42697343
        , ADFS_MAGIC3 = 0xef0110fe
        , ADFS_CLEAN  = 0xadff454e
        , ADFS_DIRTY  = 0xadfcbaad 
    };

    enum FLAGS {
        INODE_IN_USE        = 0x00000001
        , ATTR_INODE        = 0x00000004
        , INODE_LOGGED      = 0x00000008 
        , INODE_DELETED     = 0x00000010 
        , PERMANENT_FLAGS   = 0x0000ffff
        , INODE_NO_CACHE    = 0x00010000
        , INODE_WAS_WRITTEN = 0x00020000
        , NO_TRANSACTION    = 0x00040000
      };
}

#endif // CONSTANTS_H
