//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "element.h"

using namespace adcontrols;
using namespace adcontrols::internal;

Element::Element()
{
}

Element::Element( const Element& t ) : symbol_(t.symbol_)
									 , name_(t.name_)
									 , atomicNumber_(t.atomicNumber_)
									 , valence_(t.valence_)
									 , isotopes_(t.isotopes_)
{
}

Element::Element( const std::wstring& symbol
				 , const std::wstring& name
				 , int atomicNumber
				 , int valence ) : symbol_(symbol)
				                 , name_(name)
								 , atomicNumber_(atomicNumber)
								 , valence_(valence)
{
}

const std::wstring& 
Element::symbol() const
{
  return symbol_;
}

const std::wstring& 
Element::name() const
{
  return name_;
}

int 
Element::atomicNumber() const
{
  return atomicNumber_;
}

int 
Element::valence() const
{
  return valence_;
}

size_t 
Element::isotopeCount() const
{
  return isotopes_.size();
}

const Element::Isotope& 
Element::operator [] (int idx) const
{
  return isotopes_[idx];
}

void
Element::addIsotope( const Element::Isotope& isotope )
{
	isotopes_.push_back( isotope );
}

/**
**/

SuperAtom::SuperAtom()
{
}

SuperAtom::SuperAtom( const SuperAtom& t ) : name_(t.name_)
                                           , alias_(t.alias_)
										   , formula_(t.formula_)
										   , valence_(t.valence_)
{
}

SuperAtom::SuperAtom( const std::wstring& name
					 , const std::wstring& alias
					 , const std::wstring& formula
					 , int valence ) : name_(name)
                                     , alias_(alias)
									 , formula_(formula)
									 , valence_(valence)
{
}
