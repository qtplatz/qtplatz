// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
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

#include "element.hpp"

using namespace adcontrols;

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
