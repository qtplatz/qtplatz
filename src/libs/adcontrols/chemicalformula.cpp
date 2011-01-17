//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

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
            ChemicalFormulaImpl( SACONTROLSLib::ISAElementIO * );
            double getMonoIsotopicMass( const std::wstring& formula );
            double getChemicalMass( const std::wstring& formula );
            std::wstring standardFormula( const std::wstring& formula );

            CComPtr<SACONTROLSLib::ISAElementIO> pi_;

        private:
            bool initialize();
        };
    }
}

ChemicalFormula::ChemicalFormula( SACONTROLSLib::ISAElementIO * pi ) : impl_( new internal::ChemicalFormulaImpl( pi ) )
{
}

ChemicalFormula::~ChemicalFormula(void)
{
    delete impl_;
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


