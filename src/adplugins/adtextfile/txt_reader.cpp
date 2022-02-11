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

#include "txt_reader.hpp"
#include <adportable/csv_reader.hpp>
#include <adportable/debug.hpp>


using namespace adtextfile;

txt_reader::~txt_reader()
{
}

txt_reader::txt_reader()
{
}

std::array< bool, 4 >
txt_reader::load( std::ifstream& istrm
                  , data_type& data
                  , size_t skipLines
                  , std::vector< size_t >&& ignColumns
                  , bool hasTime
                  , bool hasMass
                  , bool isCentroid ) const
{
    adportable::csv::csv_reader reader( std::move( istrm ) );
    // reader.skip( skipLines );

    bool hasColor( false );
    size_t ncols = 1;
    if ( hasTime )
        ++ncols;
    if ( hasMass )
        ++ncols;
    assert( ncols > 1 );

    data.clear();
    adportable::csv::list_type list;
    size_t row( 0 );
    while ( reader.read( list ) ) {
        for ( const auto& idx: ignColumns ) {
            list.erase( list.begin() + idx );
        }
        if ( row++ == 0 ) {
            if ( ncols == 3 && list.size() >= 4 && isCentroid ) {
                hasColor = true;
                ncols = 4;
            }
        }
        switch( ncols ) {
        case 2: {
            auto datum = adportable::csv::to_tuple< double, double >( list );
            if ( hasTime ) {
                data.emplace_back( std::get<0>(datum),    0, std::get<1>(datum), 0 ); // time, intensity
            } else {
                data.emplace_back( 0,    std::get<0>(datum), std::get<1>(datum), 0 ); // mass, intensity
            }
        }
            break;
        case 3:  {
            auto datum = adportable::csv::to_tuple< double, double, double >( list ); // time, mass, intensity
            data.emplace_back( std::get<0>(datum), std::get<1>(datum), std::get<2>( datum ), 0 );
        }
            break;
        case 4:  {
            auto datum = adportable::csv::to_tuple< double, double, double, int >( list );
            data.emplace_back( datum );
        }
            break;
        } // switch
    }
    return { hasTime, hasMass, true, hasColor };
}

namespace adtextfile {
#if defined __cpp_fold_expressions
    template<class Tuple, std::size_t... Is>
    void to_legacy_impl( Tuple& dst
                         , const txt_reader::data_type& src
                         , const txt_reader::flags_type& flags
                         , std::index_sequence<Is...>)
    {
        ((
            std::get<Is>(dst).resize( flags[Is] ? src.size() : 0 )
            , std::transform( src.begin(), flags[Is] ? src.end() : src.begin(), std::get< Is >( dst ).begin(), [](const auto& t){ return std::get< Is >(t); } )
            ), ...);
    }

    template<typename... Args> void to_legacy( std::tuple< Args... >& dst
                                               , const txt_reader::data_type& src
                                               , const std::array< bool, 4 >& flags )
    {
        to_legacy_impl( dst, src, flags, std::index_sequence_for<Args...>{} );
    }
#endif
}

legacy::data_type
txt_reader::make_legacy( const data_type& src, const txt_reader::flags_type& flags ) const
{
#if defined __cpp_fold_expressions
    legacy::data_type dst;
    adtextfile::to_legacy( dst, src, flags );
#else
# if 0
    if ( flags[ 0 ] ) {
        constexpr int id = 0;
        std::get< id >( dst ).resize( src.size() );
        std::transform( src.begin(), src.end(), std::get< id >( dst ).begin(), [](const auto& s){ return std::get< id >(s); } );
    }
    if ( flags[ 1 ] ) {
        constexpr int id = 1;
        std::get< id >( dst ).resize( src.size() );
        std::transform( src.begin(), src.end(), std::get< id >( dst ).begin(), [](const auto& s){ return std::get< id >(s); } );
    }
    if ( flags[ 2 ] ) {
        constexpr int id = 2;
        std::get< id >( dst ).resize( src.size() );
        std::transform( src.begin(), src.end(), std::get< id >( dst ).begin(), [](const auto& s){ return std::get< id >(s); } );
    }
    if ( flags[ 3 ] ) {
        constexpr int id = 0;
        std::get< id >( dst ).resize( src.size() );
        std::transform( src.begin(), src.end(), std::get< id >( dst ).begin(), [](const auto& s){ return std::get< id >(s); } );
    }
# endif
#endif
    return dst;
}
