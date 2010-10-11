// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <boost/smart_ptr.hpp>
#include <string>

namespace SACONTROLSLib {
    struct ISAElementIO;
}

namespace adcontrols {

    class TableOfElements;

    namespace internal {
        class ChemicalFormulaImpl;
    }

    class ADCONTROLSSHARED_EXPORT ChemicalFormula {

        friend TableOfElements;

        ChemicalFormula( SACONTROLSLib::ISAElementIO * );

    public:
        ChemicalFormula( const ChemicalFormula& );
        ~ChemicalFormula(void);

        double getMonoIsotopicMass( const std::wstring& formula );
        double getChemicalMass( const std::wstring& formula );
    private:
        internal::ChemicalFormulaImpl * impl_;
    };
}
