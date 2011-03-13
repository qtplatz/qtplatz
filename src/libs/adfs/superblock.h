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

#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include "constants.h"
#include <boost/interprocess/file_mapping.hpp>

namespace adfs {

    namespace internal {

        typedef struct block_run {
            unsigned long allocation_group;
            unsigned short start;
            unsigned short len;
        } block_run;

        typedef struct disk_super_block {
            char name[MAX_NAME_LEN]; // 256
            int32 magic1;
            int32 fs_byte_order;
            uint32 block_size;
            uint32 block_shift;
            off_t num_blocks;
            off_t used_blocks;
            int32 inode_size;

            int32 magic2;
            int32 blocks_per_ag;
            int32 ag_shift;
            int32 num_ags;
            int32 flags;

            block_run log_blocks;
            off_t log_start;
            off_t log_end;

            int32 magic3;
            inode_addr root_dir;
            inode_addr indices;
            int32 pad[8];

            ~disk_super_block();
            disk_super_block();
        } disk_super_block;

    }

}

#endif // SUPERBLOCK_H
