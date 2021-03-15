// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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
#if ! Boost_USE_STATIC_LIBS
# define BOOST_TEST_DYN_LINK
#endif

#include <iostream>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/debug.hpp>

int
main( int argc, char * argv[] )
{
    std::string str;

    while (std::getline(std::cin, str))  {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;
        do {
            ADDEBUG() << "## neutralize: " << adcontrols::ChemicalFormula::neutralize( str );
            ADDEBUG() << "## format:     " << adcontrols::ChemicalFormula::formatFormulae( str );
        } while ( 0 );
    }
}
