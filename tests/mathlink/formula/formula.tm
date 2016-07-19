
double monoIsotopicMass P(( const char * ));

:Begin:
:Function:       monoIsotopicMass
:Pattern:        MonoIsotopicMass[i_String]
:Arguments:      { i }
:ArgumentTypes:  { String }
:ReturnType:     Real64
:End:

:Evaluate: MonoIsotopicMass::usage = "MonoIsotopicMass[x] gives the mono isotopic mass for formula x."

:Begin:
:Function:       standardFormula
:Pattern:        StandardFormula[i_String]
:Arguments:      { i }
:ArgumentTypes:  { String }
:ReturnType:     Manual
:End:

:Evaluate: StandardFormula::usage = "StandardFormula[x] gives the normalized chemical formula for given formula x."

:Begin:
:Function:       isotopeCluster
:Pattern:        IsotopeCluster[i_String, j_Real]
:Arguments:      { i, j }
:ArgumentTypes:  { String, Real64 }
:ReturnType:     Manual
:End:

:Evaluate: StandardFormula::usage = "StandardFormula[x] gives the normalized chemical formula for given formula x."
