/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

namespace adwidgets {
    namespace radiobuttons {

        // suppose to use with
        // std::tuple< QRadiobutton *, QRadioButton *, ... >
        // tied with enum, such as ion_polarity
        // setChecked ( std::tuple< QRadioButton *, ... >, positive_polarity );

        namespace detail {
            template<typename Tuple, std::size_t... Is>
            void setCheckedImpl( Tuple& t, size_t idx, std::index_sequence<Is...> ) {
                (( (Is == idx)? std::get<Is>(t)->setChecked(true) : std::get<Is>(t)->setChecked(false)),...);
            }

            template<typename Tuple, typename... Args, std::size_t... Is>
            Tuple checkedStatesImpl( std::tuple< Args... > args, std::index_sequence<Is...> ) {
                Tuple r;
                ((std::get<Is>( r ) = std::get<Is>(args)->isChecked() ),...);
                return r;
            }
        }

        template<typename... Args>
        void setChecked( std::tuple< Args...> args, size_t idx ) {
            detail::setCheckedImpl( args, idx, std::index_sequence_for< Args... >{} );
        }

        template<typename Tuple, typename... Args>
        Tuple checkedStates( std::tuple< Args... > args, Tuple&& ) {
            return detail::checkedStatesImpl<Tuple>( args, std::index_sequence_for< Args... >{} );
        }
    }
}
