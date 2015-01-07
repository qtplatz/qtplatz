/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include <cstdint>
#include <string>
#include <vector>

namespace shrader {

    class lrptic {
        enum {
            master_recored_type = 38
            , block_record_type = 6
            , master_data_size = 208
            , block_data_size = 804
        };

    public:
        ~lrptic();
        lrptic( std::istream& in, size_t fsize );
        inline operator bool () const { return loaded_; }
        
        struct TIC {
            int32_t time; // ms
            int32_t intensity;
            int32_t ptr;
            int32_t overload;
        };

        int32_t flags() const;
        int32_t nextptr() const;
        const std::vector< int32_t >& ptrs() const;
        const std::vector< TIC >& tic() const;
    private:
        bool loaded_;
        int32_t flags_;
        int32_t nextptr_;
        std::vector< TIC > tic_;
    };

}

