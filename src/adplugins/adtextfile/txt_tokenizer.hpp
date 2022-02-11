// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include <array>
#include <memory>
#include <vector>
#include <tuple>

namespace adtextfile {

    class txt_tokenizer {
    public:
        typedef std::tuple< std::vector< double >    // time
                            , std::vector< double >  // mass
                            , std::vector< double >  // intensity
                            , std::vector< uint8_t > // color
                            > data_type;
        typedef std::array< bool, 4 > flags_type;

        ~txt_tokenizer();
        txt_tokenizer();
        flags_type load( std::ifstream&
                         , data_type&
                         , size_t skipLines
                         , std::vector< size_t >&& ignColumns
                         , bool hasTime
                         , bool hasMass
                         , bool isCentroid ) const;
    };

}
