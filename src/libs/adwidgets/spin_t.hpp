/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef SPIN_TYPE_HPP
#define SPIN_TYPE_HPP

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <adportable/debug.hpp>

namespace adwidgets {

    template<class Spin, typename T> struct [[deprecated("use spin_init function template")]] spin_t;

    template<class Spin, typename T = double> struct spin_t {
        static void init( Spin * s, T minimum, T maximum, T initval ) {
            s->setMinimum( minimum );
            s->setMaximum( maximum );
            s->setValue( initval );
            s->setKeyboardTracking( false );
        }
    };

    namespace spin_initializer {
        struct Alignment    { typedef Qt::AlignmentFlag value_type; value_type value; Alignment ( value_type t ) : value( t ){}  };
        struct Decimals     { typedef int value_type;               value_type value; Decimals  ( value_type t ) : value( t ){}  };
        template<typename value_type = double > struct Minimum    { value_type value; Minimum   ( value_type t ) : value( t ){}  };
        template<typename value_type = double > struct Maximum    { value_type value; Maximum   ( value_type t ) : value( t ){}  };
        template<typename value_type = double > struct Value      { value_type value; Value     ( value_type t ) : value( t ){}  };
        template<typename value_type = double > struct SingleStep { value_type value; SingleStep( value_type t ) : value( t ){}  };

        template< class Spin > struct spin_type {
            template< typename T > static void assign_to( Spin * spin, const T& t ) {
                ADDEBUG() << "\tError: not assigned for " << typeid(T).name() << ", " << t.value;
            }
        };

        // QDoubleSpinBox
        template<> template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Decimals&  t );
        template<> template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Minimum<>& t );
        template<> template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Maximum<>& t );
        template<> template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Value<>&   t );
        template<> template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const SingleStep<>& t );
        template<> template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Alignment& t );

        // QSpinBox
        template<> template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const Minimum<int>& t );
        template<> template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const Maximum<int>& t );
        template<> template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const Value<int>&   t );
        template<> template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const SingleStep<int>& t );
        template<> template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const Alignment& t );

        template< class Spin, typename Tuple, std::size_t... Is >
        void spin_init_impl( Spin * spin, Tuple&& args, std::index_sequence< Is... > ) {
            (( spin_type<Spin>::assign_to( spin, std::get<Is>( args ))) , ... );
        }

        template< class Spin, typename... Args >
        void spin_init( Spin * spin, std::tuple< Args... >&& args ) {
            spin_init_impl( spin, args, std::index_sequence_for< Args... >{} );
        }
    }
}

#endif // SPIN_TYPE_HPP
