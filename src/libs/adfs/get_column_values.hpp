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

#include <tuple>
#include "sqlite.hpp"

namespace adfs {

    template<typename Tuple, std::size_t... Is> Tuple get_column_values_impl( Tuple& t, adfs::stmt& sql, std::index_sequence<Is...> ) {
        ((std::get<Is>(t) = sql.get_column_value< std::tuple_element_t<Is, Tuple> >( Is )), ...);
        return t;
    }

    template<typename... Args> std::tuple< Args... > get_column_values( adfs::stmt& sql ) {
        std::tuple<Args...> t;
        return get_column_values_impl( t, sql, std::index_sequence_for< Args... >{} );
    }

}
