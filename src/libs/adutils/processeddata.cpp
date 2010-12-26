//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "processeddata.h"

#include <adcontrols/massspectrum.h>
#include <adcontrols/chromatogram.h>
#include <adcontrols/processmethod.h>
#include <adcontrols/elementalcompositioncollection.h>

using namespace adutils;

ProcessedData::ProcessedData()
{
}

ProcessedData::value_type
ProcessedData::toVariant( boost::any & a )
{
    if ( a.type() == typeid( MassSpectrumPtr ) )
        return boost::any_cast< MassSpectrumPtr >( a );

    else if ( a.type() == typeid( ChromatogramPtr ) )
        return boost::any_cast< ChromatogramPtr >( a );

    else if ( a.type() == typeid( ProcessMethodPtr ) )
        return boost::any_cast< ProcessMethodPtr >( a );

    else if ( a.type() == typeid( ElementalCompositionCollectionPtr ) )
        return boost::any_cast< ElementalCompositionCollectionPtr >( a );

    return Nothing();
}