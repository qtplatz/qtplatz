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

#include "spin_t.hpp"

namespace adwidgets {

    namespace spin_initializer {
        template<>
        template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Decimals& t ) {
            spin->setDecimals( t.value );
        }

        template<>
        template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Minimum<double>& t )
        {
            spin->setMinimum( t.value );
        }

        template<>
        template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Maximum<double>& t )
        {
            spin->setMaximum( t.value );
        }

        template<>
        template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Value<double>& t )
        {
            spin->setValue( t.value );
        }

        template<>
        template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const SingleStep<double>& t )
        {
            spin->setSingleStep( t.value );
        }

        template<>
        template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Minimum<int>& t )
        {
            spin->setMinimum( t.value );
        }

        template<>
        template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Maximum<int>& t )
        {
            spin->setMaximum( t.value );
        }

        template<>
        template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const Value<int>& t )
        {
            spin->setValue( t.value );
        }

        template<>
        template<> void spin_type< QDoubleSpinBox >::assign_to( QDoubleSpinBox * spin, const SingleStep<int>& t )
        {
            spin->setSingleStep( t.value );
        }

        ///////////////////////////////////
        template<>
        template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const Minimum<double>& t )
        {
            spin->setMinimum( t.value );
        }

        template<>
        template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const Maximum<double>& t )
        {
            spin->setMaximum( t.value );
        }

        template<>
        template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const Value<double>& t )
        {
            spin->setValue( t.value );
        }

        template<>
        template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const SingleStep<double>& t )
        {
            spin->setSingleStep( t.value );
        }

        template<>
        template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const Minimum<int>& t )
        {
            spin->setMinimum( t.value );
        }

        template<>
        template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const Maximum<int>& t )
        {
            spin->setMaximum( t.value );
        }

        template<>
        template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const Value<int>& t )
        {
            spin->setValue( t.value );
        }

        template<>
        template<> void spin_type< QSpinBox >::assign_to( QSpinBox * spin, const SingleStep<int>& t )
        {
            spin->setSingleStep( t.value );
        }
    }
}
