// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "adcontrols_global.h"

// interface for an element in 'table-of-element' 

namespace adcontrols {

    class TableOfElement;
	namespace toe { class isotopes; struct isotope; }
	namespace detail { struct element; }

    namespace mol {

        // class implements in "tableofelement.cpp"
        
        class ADCONTROLSSHARED_EXPORT element {

        public:
            element( const detail::element * p = 0 );
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


