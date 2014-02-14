// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

// #include <boost/serialization/nvp.hpp>
// #include <boost/serialization/version.hpp>
// #include <boost/serialization/string.hpp>
// #include <boost/serialization/vector.hpp>
#include "adcontrols_global.h"
//#include <string>
//#include <compiler/disable_dll_interface.h>

namespace adcontrols {

	namespace toe { class isotopes; struct isotope; }
	namespace detail { struct element; }

    // 'element' is small & fast access interface for table-of-element
    // implimented for quick isotope cluster pattern calculation using c++11 range patterns

    namespace mol {

        class ADCONTROLSSHARED_EXPORT element {
            friend class TableOfElement;
            element( const detail::element * );
        public:
            element( const element& );
            
            operator bool () const;
            const char * symbol() const;
            const char * name() const;
            int atomicNumber() const;
            int valence() const;
            toe::isotopes isotopes() const;
            int count() const;
            void count( int );
            static double monoIsotopicMass( const element&, int isotope = 0 );
            static double chemicalMass( const element& );
        private:
            const detail::element * p_;
            int count_;
        };

    } // namespace mol

}

//BOOST_CLASS_VERSION(adcontrols::Element, 1)
//BOOST_CLASS_VERSION(adcontrols::Element::Isotope, 1)
//BOOST_CLASS_VERSION(adcontrols::SuperAtom, 1)

