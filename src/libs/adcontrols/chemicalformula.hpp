// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
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
#include <boost/optional.hpp>
#include <string>
#include <map>
#include <vector>

namespace adcontrols {

    class TableOfElement;
    class CTable;
    namespace mol { class element; class molecule; }

    namespace cf {
        struct Charge { typedef int value_type; value_type value; Charge( const value_type& t = 0 ) : value{t} {}; };
        struct RichText { typedef bool value_type; value_type value; RichText( const value_type& t = 0 ) : value{t} {}; };
        struct MolSubst { typedef std::string value_type; value_type value; MolSubst( const value_type& t = {} ) : value{t}{} };

        typedef std::tuple< Charge, RichText, MolSubst > format_option_type; // charge, richText, mol replace
    };

    class ADCONTROLSSHARED_EXPORT ChemicalFormula {
    public:
        ~ChemicalFormula();
        ChemicalFormula();
        ChemicalFormula( const ChemicalFormula& );

		typedef std::map< std::string, size_t > elemental_composition_map_t;

        double getMonoIsotopicMass( const std::wstring& formula, bool handleCharge = true ) const;
		double getMonoIsotopicMass( const std::string& formula, bool handleCharge = true ) const;
        std::pair< double, int > getMonoIsotopicMass( const std::vector< std::pair< std::string, char > >& formulae, int charge = 0 ) const;

        double getChemicalMass( const std::wstring& formula ) const;
		double getChemicalMass( const std::string& formula ) const;
        double getElectronMass() const;

		static std::wstring standardFormula( const std::wstring& formula, bool removeChare = false );
		static std::string standardFormula( const std::string& formula, bool removeCharge = false );
		static std::pair< std::string, int > standardFormula( const std::vector< std::pair< std::string, char > >& formulae ); // formula, charge
        [[deprecated("use toMoleclue")]]
        static bool getComposition( std::vector< mol::element >&, const std::string& formula, int& charge );

        static mol::molecule toMolecule( const std::string& formula );
        static mol::molecule toMolecule( const std::string& formula, const std::string& adduct );
        static mol::molecule toMolecule( const std::vector< std::pair< std::string, char > >& formulae, int charge = 0 );

        static std::wstring formatFormula( const std::wstring& formula, bool richText = true );
        static std::string formatFormula( const std::string& formula, bool richText = true );

        /*
         * split formula followed by a list of adducts/losses,
         * ex. 'CH3(C2H4)5OH +H +Na +NH3 -C2H4' will return pair(' ', "CH3(C2H4)50H"), pair('+' "Na"), pair('+' "NH3"), pair( '-' "C2H4")
         */
        typedef std::pair< std::string, char > formula_adduct_t;
        static std::vector< formula_adduct_t > split( const std::string& adducts );
        static std::pair< std::string, int > neutralize( const std::string& formula ); // remove charged form, [H]+ --> H
        static size_t number_of_atoms( const std::string& formula );   // return number of all atoms
        static size_t number_of_atoms( const std::string& formula, const char * atom );   // return number of specified atom

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
        static std::string formatFormulae( const std::string& formula, int charge, bool richText );
        static std::string formatFormulae( const std::string& formula, cf::format_option_type&& );

        static std::string formatAdduct( const std::string& adduct_formula, const char * blacket = "[]");

        /**
         * makeFormulae synthesize standard formulae from formula and commna (or semicolon) separated list of adducts/lose
         * expecting addcuts ex: +H,+CH3CN,-COOH,...
         */
        // static std::vector< std::pair< std::string, char > > splitAdducts( const std::string& adducts );
        static std::vector< std::string > standardFormulae( const std::string& formula, const std::string& adducts );
        static std::vector< std::string > standardFormulae( const std::string& formula, const std::string& adducts, std::vector< std::string >& adductlist );

        static std::string make_adduct_string( const std::vector< std::pair< std::string, char > >& );  // collect adducts/losses
        static std::string make_formula_string( const std::vector< std::pair< std::string, char > >& ); // exclude adducts/losses
    };
}
