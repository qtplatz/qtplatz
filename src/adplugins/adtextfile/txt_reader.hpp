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

#include <memory>
#include <vector>
#include <tuple>

namespace adtextfile {

    enum {
        flag_time        // 0
        , flag_mass      // 1
        , flag_intensity // 2
        , flag_color     // 3
    } flag_pos;

    namespace legacy {
        typedef std::tuple< std::vector< double >    // time
                            , std::vector< double >  // mass
                            , std::vector< double >  // intensity
                            , std::vector< int >     // color
                            > data_type;
    }

    class txt_reader {
    public:
        typedef std::tuple< double, double, double, int > datum_type;
        typedef std::vector< datum_type > data_type;
        typedef std::array< bool, 4 > flags_type;
        ~txt_reader();
        txt_reader();

        flags_type load( std::ifstream&
                         , data_type&
                         , size_t skipLines
                         , std::vector< size_t >&& ignColumns
                         , bool hasTime
                         , bool hasMass
                         , bool isCentroid ) const;

        legacy::data_type make_legacy( const data_type& data, const std::array< bool, 4 >& flags ) const;
    };

    namespace datum {
        inline double time( const txt_reader::datum_type& datum )      { return std::get< 0 >( datum ); };
        inline double mass( const txt_reader::datum_type& datum )      { return std::get< 1 >( datum ); };
        inline double intensity( const txt_reader::datum_type& datum ) { return std::get< 2 >( datum ); };
        inline int color( const txt_reader::datum_type& datum )        { return std::get< 3 >( datum ); };
    }

}
