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

#include "chemicalformula.h"
#include "tableofelements.h"
#include "import_sacontrols.h"
#include <boost/noncopyable.hpp>

using namespace adcontrols;

namespace adcontrols {
    namespace internal {
        class ChemicalFormulaImpl {
            SACONTROLSLib::ISAFormulaDataPtr piFormulaData;
            SACONTROLSLib::ISAFormulaParserPtr piFormulaParser;

        public:
            ChemicalFormulaImpl( SACONTROLSLib::ISAElementIO * p = 0 );
            double getMonoIsotopicMass( const std::wstring& formula );
            double getChemicalMass( const std::wstring& formula );
            std::wstring standardFormula( const std::wstring& formula );

            CComPtr<SACONTROLSLib::ISAElementIO> pi_;

        private:
            bool initialize();
        };
    }
}

ChemicalFormula::~ChemicalFormula(void)
{
    impl_->pi_.Release();
    delete impl_;
}

ChemicalFormula::ChemicalFormula() : impl_( new internal::ChemicalFormulaImpl )
{
    impl_->pi_ = static_cast<SACONTROLSLib::ISAElementIO *>( *TableOfElements::instance() );
}

double
ChemicalFormula::getMonoIsotopicMass( const std::wstring& formula )
{
    return impl_->getMonoIsotopicMass( formula );
}

double
ChemicalFormula::getChemicalMass( const std::wstring& formula )
{
    return impl_->getChemicalMass( formula );
}

std::wstring
ChemicalFormula::standardFormula( const std::wstring& formula )
{
    return impl_->standardFormula( formula );
}

///////////////
using namespace adcontrols::internal;

ChemicalFormulaImpl::ChemicalFormulaImpl( SACONTROLSLib::ISAElementIO * pi ) : pi_( pi )
{
}

double
ChemicalFormulaImpl::getChemicalMass( const std::wstring& formula )
{
    if ( initialize() ) {
        if ( piFormulaParser->Parse( pi_, formula.c_str(), 0, 0, piFormulaData ) == S_OK ) {
            double mass = piFormulaData->ChemicalMass;
            return mass;
        }
    }
    return 0;
}

double
ChemicalFormulaImpl::getMonoIsotopicMass( const std::wstring& formula )
{
    if ( initialize() ) {
        if ( piFormulaParser->Parse( pi_, formula.c_str(), 0, 0, piFormulaData ) == S_OK ) {
            double mass = piFormulaData->MonoisotopicMass;
            return mass;
        }
    }
    return 0;
}

std::wstring
ChemicalFormulaImpl::standardFormula( const std::wstring& formula )
{
    if ( initialize() ) {
        if ( piFormulaParser->Parse( pi_, formula.c_str(), 0, 0, piFormulaData ) == S_OK ) {
            _bstr_t str = piFormulaData->StandardFormula;
            return static_cast< wchar_t * >( str );
        }
    }
    return L"";
}

bool
ChemicalFormulaImpl::initialize()
{
    if ( ! piFormulaParser ) {
        if ( piFormulaParser.CreateInstance( SACONTROLSLib::CLSID_SAFormulaParser ) != S_OK )
            return false;
    }
    if ( ! piFormulaData ) {
        if ( piFormulaData.CreateInstance( SACONTROLSLib::CLSID_SAFormulaData ) != S_OK )
            return false;
    }
    return true;
}


