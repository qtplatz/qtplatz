// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <boost/any.hpp>
#include <boost/variant.hpp>
#include <boost/smart_ptr.hpp>

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
    class ProcessMethod;
    class ElementalCompositionCollection;
}

namespace adutils {

    typedef boost::shared_ptr< adcontrols::MassSpectrum > MassSpectrumPtr;
    typedef boost::shared_ptr< adcontrols::Chromatogram > ChromatogramPtr;
    typedef boost::shared_ptr< adcontrols::ProcessMethod > ProcessMethodPtr;
    typedef boost::shared_ptr< adcontrols::ElementalCompositionCollection > ElementalCompositionCollectionPtr;

    class ProcessedData {
    public:
        ProcessedData();

        class Nothing { 
        public:
            Nothing() {}
        };

        typedef boost::variant< Nothing
                              , MassSpectrumPtr
                              , ChromatogramPtr
                              , ProcessMethodPtr
                              , ElementalCompositionCollectionPtr 
                              > value_type;

        static value_type toVariant( boost::any& );

    private:
        value_type datum_;
    };

}

