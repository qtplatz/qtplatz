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

#ifndef INODE_H
#define INODE_H

#include "constants.h"

namespace adfs {
    namespace internal {

        typedef struct adfs_inode {
            int32 magic1;
            inode_addr inode_num;
            int32 uid;
            int32 gid;
            int32 mode;
            int32 flags;
            time_t create_time;
            time_t modified_time;
            inode_addr parent;
            inode_addr attributes;
            uint32 type;
            int32 inode_size;
            // binode_etc * etc;
            // data_stream data;
            int32 pad[4];
            int32 small_data[1];

        } adfs_inode;

        class inode {
        public:
            inode();
        };

    } // internal
}

#endif // INODE_H
