// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#pragma once

#include <tuple>
#include <adportable/is_type.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

/////////
// std::tuple to boost::variant
// find if boost::any contains a type in a tuple, then return it as boost::variant
//
// see plugins/dataproc/chromatogramwnd.cpp for usage
//
namespace adutils {

    struct null_t {};

    template < typename ... T >  struct type_list_t {};

    template< typename Last >  struct type_list_t< Last > {
        template< typename variant_type >
        boost::optional<variant_type> do_cast( variant_type&&, boost::any& a ) const {
            return {};
        }
    };

    template< typename First, typename... Args >  struct type_list_t< First, Args... > {
        template< typename variant_type >
        boost::optional<variant_type> do_cast( variant_type&&, boost::any& a ) const {
            if ( adportable::a_type< First >::is_a( a ) )
                return variant_type( boost::any_cast< First >( a ) );
            return type_list_t< Args... >{}.do_cast( variant_type{}, a );
        }
    };

    template< typename Tuple > struct to_variant;

    template< typename... Args >
    struct to_variant< std::tuple<Args ... > > {
        using type = boost::variant< Args ... >;
        boost::optional<type> operator()( boost::any& a ) const {
            return type_list_t< Args ..., null_t >{}.do_cast( type{}, a );
        }
    };
    ////////////
}
