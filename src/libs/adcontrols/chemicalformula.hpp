// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#pragma once

#include "adcontrols_global.h"
#include <string>
#include <map>
#include <vector>

namespace adcontrols {

    class TableOfElement;
    class CTable;
    namespace mol { class element; }

    class ADCONTROLSSHARED_EXPORT ChemicalFormula {
    public:
        ~ChemicalFormula();
        ChemicalFormula();
        ChemicalFormula( const ChemicalFormula& );

		typedef std::map< std::string, size_t > elemental_composition_map_t;

        double getMonoIsotopicMass( const std::wstring& formula ) const;
		double getMonoIsotopicMass( const std::string& formula ) const;
        // double getMonoIsotopicMass( const std::wstring& formula, const std::pair< std::wstring, std::wstring >& ) const;
        // double getMonoIsotopicMass( const std::string& formula, const std::pair< std::string, std::string >& ) const;
        double getMonoIsotopicMass( const std::vector< std::pair< std::string, char > >& formulae ) const;        

        double getChemicalMass( const std::wstring& formula ) const;
		double getChemicalMass( const std::string& formula ) const;
        double getElectronMass() const;
        
		static std::wstring standardFormula( const std::wstring& formula );
		static std::string standardFormula( const std::string& formula );
		static std::string standardFormula( const std::vector< std::pair< std::string, char > >& formulae );
        static bool getComposition( std::vector< mol::element >&, const std::string& formula );
        static std::wstring formatFormula( const std::wstring& formula, bool richText = true );
        static std::string formatFormula( const std::string& formula, bool richText = true );
        
        /*
         * split formula followed by a list of adducts/losses, 
         * ex. 'CH3(C2H4)5OH +H +Na +NH3 -C2H4' will return pair(' ', "CH3(C2H4)50H"), pair('+' "Na"), pair('+' "NH3"), pair( '-' "C2H4")
         */
        typedef std::pair< std::string, char > formula_adduct_t;
        static std::vector< formula_adduct_t > split( const std::string& adducts );

#if defined _MSC_VER
        static const char sign_formula = '\0';
        static const char sign_adduct = '+';
        static const char sign_loss = '-';
#else
        static constexpr char sign_formula = '\0';
        static constexpr char sign_adduct = '+';
        static constexpr char sign_loss = '-';
#endif
        static bool is_molformula( formula_adduct_t t ) { return t.second == sign_formula; }
        static bool is_adduct( formula_adduct_t t ) { return t.second == sign_adduct; }
        static bool is_loss( formula_adduct_t t ) { return t.second == sign_loss; }

        static std::string formatFormulae( const std::string& formula, bool richText = true );
        static std::wstring formatFormulae( const std::wstring& formula, bool richText = true );

        /**
         * makeFormulae synthesize standard formulae from formula and commna (or semicolon) separated list of adducts/lose
         * expecting addcuts ex: +H,+CH3CN,-COOH,...
         */
        static std::vector< std::pair< std::string, char > > splitAdducts( const std::string& adducts );
        static std::vector< std::string > standardFormulae( const std::string& formula, const std::string& adducts );
        static std::vector< std::string > standardFormulae( const std::string& formula, const std::string& adducts, std::vector< std::string >& adductlist );

        static std::string make_adduct_string( const std::vector< std::pair< std::string, char > >& );  // collect adducts/losses
        static std::string make_formula_string( const std::vector< std::pair< std::string, char > >& ); // exclude adducts/losses
    };
}
