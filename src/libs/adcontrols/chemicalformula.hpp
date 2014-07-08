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

        double getChemicalMass( const std::wstring& formula ) const;
		double getChemicalMass( const std::string& formula ) const;
        double getElectronMass() const;
        
		static std::wstring standardFormula( const std::wstring& formula );
		static std::string standardFormula( const std::string& formula );
        static bool getComposition( std::vector< mol::element >&, const std::string& formula );
        static std::wstring formatFormula( const std::wstring& formula, bool richText = true );
        static std::string formatFormula( const std::string& formula, bool richText = true );
        
        static bool split( const std::string& formula
                           , std::vector< std::string >&
                           , const char * dropped_delims = "", const char * kept_delims = "+-" );

        static std::string formatFormulae( const std::string& formula, const char * delims = "+-", bool richText = true );
    };
}
