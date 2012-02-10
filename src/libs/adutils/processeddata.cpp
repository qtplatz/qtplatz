// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "processeddata.hpp"

#include <adcontrols/massspectrum.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/elementalcompositioncollection.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adportable/debug.hpp>

using namespace adutils;

ProcessedData::ProcessedData()
{
}

ProcessedData::value_type
ProcessedData::toVariant( boost::any & a )
{
// See issue on boost.  https://svn.boost.org/trac/boost/ticket/754
#if defined __GNUC__ 
    static const char * type_name[] = {
        typeid( MassSpectrumPtr ).name()
        , typeid( ChromatogramPtr ).name()
        , typeid( ElementalCompositionCollectionPtr ).name()
        , typeid( ProcessMethodPtr ).name()
        , typeid( MSCalibrateResultPtr ).name()
    };

    std::string atype = a.type().name();
    if ( atype == type_name[ 0 ] )
        return boost::any_cast< MassSpectrumPtr >( a );
    else if ( atype == type_name[ 1 ] )
        return boost::any_cast< ChromatogramPtr >( a );
    else if ( atype == type_name[ 2 ] )
        return boost::any_cast< ElementalCompositionCollectionPtr >( a );
    else if ( atype == type_name[ 3 ] )
        return boost::any_cast< ProcessMethodPtr >( a );
    else if ( atype == type_name[ 4 ] )
        return boost::any_cast< MSCalibrateResultPtr >( a );
#else
    if ( a.type() == typeid( MassSpectrumPtr ) )
        return boost::any_cast< MassSpectrumPtr >( a );

    else if ( a.type() == typeid( ChromatogramPtr ) )
        return boost::any_cast< ChromatogramPtr >( a );

    else if ( a.type() == typeid( ProcessMethodPtr ) )
        return boost::any_cast< ProcessMethodPtr >( a );

    else if ( a.type() == typeid( ElementalCompositionCollectionPtr ) )
        return boost::any_cast< ElementalCompositionCollectionPtr >( a );

    else if ( a.type() == typeid( MSCalibrateResultPtr ) )
        return boost::any_cast< MSCalibrateResultPtr >( a );
#endif

#if defined DEBUG && defined __GNUC__
    std::cerr << "toVariant lookup: " << atype << std::endl;
    for ( int i = 0; i < 4; ++i )
        std::cerr << "\t? " << type_name[i] << std::endl;
    std::cerr << std::endl;
#endif
    adportable::debug(__FILE__, __LINE__)
        << "ProcessedData::toVariant( " << a.type().name() << " ) -- return Nothing()";
    return Nothing();
}
