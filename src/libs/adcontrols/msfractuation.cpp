// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
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

#include "msfractuation.hpp"
#include "lockmass.hpp"
#include <type_traits>
#include <map>

namespace adcontrols {

    class MSFractuation::impl {
    public:
        std::map< int64_t /* rowid */, lockmass::fitter > fitter_;
    };

}

using namespace adcontrols;

MSFractuation::~MSFractuation(void)
{
}

MSFractuation::MSFractuation() : impl_( std::make_unique< impl >() )
{
    static_assert( std::is_base_of< std::enable_shared_from_this< MSFractuation >, MSFractuation >::value
                             , "MSFractuation must be decendent of std::enable_shared_from_this<>" );
}

// static
std::shared_ptr< MSFractuation >
MSFractuation::create()
{
    return std::make_shared< MSFractuation >();
}

void
MSFractuation::insert( int64_t rowid, const lockmass::fitter& fitter )
{
    impl_->fitter_[ rowid ] = fitter;
}

const lockmass::fitter
MSFractuation::find( int64_t rowid )
{
    auto it = impl_->fitter_.lower_bound( rowid );

    if ( it == impl_->fitter_.end() ) 
        return lockmass::fitter();

    if ( it == impl_->fitter_.begin() || it->first == rowid )
        return it->second;

    auto prev = it;
    --prev;
    if ( rowid - prev->first <= it->first - rowid )
        return prev->second;
    return it->second;
}

