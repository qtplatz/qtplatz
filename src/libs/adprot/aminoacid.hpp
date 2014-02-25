/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef AMINOACID_HPP
#define AMINOACID_HPP

#include "adprot_global.hpp"
#include <compiler/disable_dll_interface.h>
#include <string>

namespace adprot {

    class ADPROTSHARED_EXPORT AminoAcid  {
    public:
        AminoAcid( int symbol, const char * _3letter, const char * formula, const char * formula_H2O, const char * smiles );

        typedef AminoAcid * iterator;

        static iterator begin();
        static iterator end();
        static size_t size();
        static const AminoAcid * find( int symbol );

        int code() const;     // 1 letter
        const char * symbol() const; // 3 letter
        const char * formula( bool free = true ) const;
        const char * smiles() const;
    private:
        const int symbol_1letter_;
        const char * symbol_3letter_;
        const char * formula_;
        const char * formula_H2O_;
        const char * smiles_;
    };
}

#endif // AMINOACID_HPP
