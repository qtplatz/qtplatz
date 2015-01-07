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

#include "lrptic.hpp"
#include <array>
#include <iostream>

namespace shrader {
    namespace detail {
#pragma pack(1)
        struct TIC {
            int32_t RT;
            int32_t Tic;
            int32_t Ptr;
            int32_t Overload;
        };

        struct MasterTIC {
            int32_t flags;
            int32_t nextptr;
            int32_t TICptr[ 50 ];
        };

        struct BlockTIC {
            int32_t flags;
            TIC tic[ 50 ];
        };

#pragma pack()
    }
}

using namespace shrader;

lrptic::~lrptic()
{
}

lrptic::lrptic(std::istream& in, size_t fsize) : loaded_( false )
{
    static_assert(sizeof( detail::MasterTIC ) == lrptic::master_data_size
                  , "struct 'detail::MasterTIC' not alinged, check declaration.");

    static_assert(sizeof( detail::BlockTIC ) == lrptic::block_data_size
                  , "struct 'detail::BlockTIC' not alinged, check declaration.");

    auto pos = in.tellg();
    std::cout << "pos: " << pos << std::endl;

    detail::MasterTIC master;

    if ( ( fsize - in.tellg() ) >= master_data_size ) {

        in.read( reinterpret_cast< char * >(&master), lrptic::master_data_size );

        if ( !in.fail() && master.flags == 38 ) { // master TIC pointer block

            loaded_ = true;
            flags_ = master.flags;
            nextptr_ = ( master.nextptr - 1 ) != pos ? ( master.nextptr - 1 ) : 0;

        } else {
            in.seekg( pos );
            return;
        }
    }

    for ( int idx = 0; idx < 50 && master.TICptr[ idx ]; ++idx ) {
        // TODO: nextptr should be handled, if data consists with more than 2500 (50 * 50) points

        in.seekg( master.TICptr[ idx ] - 1 );

        if ( fsize - in.tellg() >= block_data_size ) {

            pos = in.tellg();

            std::array< char, block_data_size > opaque;
            auto block = reinterpret_cast<const detail::BlockTIC *>(opaque.data());

            in.read( opaque.data(), opaque.size() );
            if ( !in.fail() && block->flags == block_record_type ) {

                for ( int i = 0; i < 50 && block->tic[ i ].Ptr > 0; ++i ) {
                    lrptic::TIC d = { block->tic[i].RT, block->tic[i].Tic, block->tic[i].Ptr - 1, block->tic[i].Overload };
                    tic_.push_back( d );
                }
            } else {
                in.seekg( pos ); // rewind
                return;
            }
        }
    }
}

int32_t 
lrptic::flags() const
{
    return flags_;
}

int32_t 
lrptic::nextptr() const
{
    return nextptr_;
}

const std::vector< lrptic::TIC >&
lrptic::tic() const
{
    return tic_;
}
