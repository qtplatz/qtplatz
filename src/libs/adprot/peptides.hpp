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

#ifndef PEPTIDES_HPP
#define PEPTIDES_HPP

#include "adprot_global.hpp"
#include "peptide.hpp"
#include <vector>

namespace adprot {

    class ADPROTSHARED_EXPORT peptides {
    public:
        peptides();
        typedef std::vector< adprot::peptide >::size_type size_type;
        typedef std::vector< adprot::peptide >::iterator iterator;
        typedef std::vector< adprot::peptide >::const_iterator const_iterator;

        size_type size() const;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        iterator erase( iterator first, iterator last );
        peptides& operator << ( const peptide& );

    private:
        std::vector< adprot::peptide > vec_;
    };

}

#endif // PEPTIDES_HPP
