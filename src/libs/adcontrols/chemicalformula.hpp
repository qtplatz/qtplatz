// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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
#include <boost/smart_ptr.hpp>
#include <string>
#include <map>

namespace adcontrols {

    class TableOfElements;
    class CTable;

    namespace internal {
        class ChemicalFormulaImpl;
    }

    class ADCONTROLSSHARED_EXPORT ChemicalFormula {
    public:
        ~ChemicalFormula();
        ChemicalFormula();
        ChemicalFormula( const ChemicalFormula& );

		typedef std::map< const wchar_t *, size_t > elemental_composition_map_t;

        double getMonoIsotopicMass( const std::wstring& formula );
        double getChemicalMass( const std::wstring& formula );
		std::wstring standardFormula( const std::wstring& formula );
		static std::wstring getFormula( const CTable& );
		static elemental_composition_map_t getComposition( const std::wstring& formula );

    private:
        internal::ChemicalFormulaImpl * impl_;
    };
}
