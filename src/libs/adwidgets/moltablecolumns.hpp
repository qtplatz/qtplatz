/**************************************************************************
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include <QString>

namespace adwidgets {
    namespace moltable {

        struct col_formula       { typedef QString value_type; const QString header = "formula";       };
        struct col_adducts       { typedef QString value_type; const QString header = "adduct/lose";   };
        struct col_mass          { typedef double  value_type; const QString header = "mass";          };
        struct col_retentionTime { typedef double  value_type; const QString header = "t<sub>R</sub>"; };
        struct col_msref         { typedef bool    value_type; const QString header = "lock mass";     };
        struct col_protocol      { typedef int     value_type; const QString header = "protocol";      };
        struct col_synonym       { typedef QString value_type; const QString header = "synonym";       };
        struct col_memo          { typedef QString value_type; const QString header = "memo";          };
        struct col_svg           { typedef QString value_type; const QString header = "Structure";     };
        struct col_smiles        { typedef QString value_type; const QString header = "SMILES";        };
        struct col_nlaps         { typedef QString value_type; const QString header = "nlaps";         };
        struct col_abundance     { typedef double  value_type; const QString header = "R.A.";          };
        struct col_logp          { typedef double  value_type; const QString header = "logP";          };
        struct col_apparent_mass { typedef double  value_type; const QString header = "apparent <i>m/z</i>"; };
        struct col_tof           { typedef double  value_type; const QString header = "tof(&mu;s)"; };

        // following lines are use in XChromatogramsTable
        struct col_xicMethod     { typedef double  value_type; const QString header = "Method";     };
        struct col_tofWindow  { typedef double  value_type; const QString header = "Window(ns)"; };
        struct col_massWindow { typedef double  value_type; const QString header = "Window(Da)"; };
    }
}
