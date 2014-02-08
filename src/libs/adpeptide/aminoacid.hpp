/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "adpeptide_global.hpp"
#include <compiler/disable_dll_interface.h>
#include <string>

namespace adpeptide {

    class ADPEPTIDESHARED_EXPORT AminoAcid  {
    public:
        AminoAcid( char symbol );
        AminoAcid( const char * _3letter );

        class ADPEPTIDESHARED_EXPORT iterator {
            size_t pos_;
        public:
            iterator( size_t pos );
            const iterator& operator ++ () { ++pos_; return *this; }
            bool operator != ( const iterator& rhs ) const { return pos_ != rhs.pos_; }
            operator AminoAcid* () const;
        };

        static const iterator begin();
        static const iterator end();
        static size_t size();

        const std::string& symbol( bool _3letter = true ) const;
        const std::string& formula() const;
        const std::string& smiles() const;
    private:
        int symbol_;
    };
}

#endif // AMINOACID_HPP
