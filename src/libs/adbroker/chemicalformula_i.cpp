//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "session_i.h"
#include "chemicalformula_i.h"

#include <adcontrols/tableofelements.h>

using namespace adbroker;

ChemicalFormula_i::ChemicalFormula_i(void)
{
}

ChemicalFormula_i::~ChemicalFormula_i(void)
{
}

double
ChemicalFormula_i::getMonoIsotopicMass( const CORBA::WChar * formula )
{
    adcontrols::ChemicalFormula chemicalFormula = adcontrols::TableOfElements::instance()->getChemicalFormula();
    return chemicalFormula.getMonoIsotopicMass( formula );
}

double
ChemicalFormula_i::getChemicalMass( const CORBA::WChar * formula )
{
    adcontrols::ChemicalFormula chemicalFormula = adcontrols::TableOfElements::instance()->getChemicalFormula();
    return chemicalFormula.getMonoIsotopicMass( formula );
}