// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#pragma warning (disable: 4996)
# include "adinterface/brokerS.h"
#pragma warning (default: 4996)

namespace adbroker {

    class ChemicalFormula_i : public POA_Broker::ChemicalFormula {
    public:
        ChemicalFormula_i(void);
        ~ChemicalFormula_i(void);

        virtual ::CORBA::Double getMonoIsotopicMass ( const CORBA::WChar * formula );
        virtual ::CORBA::Double getChemicalMass ( const CORBA::WChar * formula );
    };
}


