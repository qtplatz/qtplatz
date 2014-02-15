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

#ifndef DIGESTEDPEPTIDES_HPP
#define DIGESTEDPEPTIDES_HPP

#include "adprot_global.hpp"
#include <compiler/disable_dll_interface.h>
#include <memory>

namespace adprot {

    class protein;
	class peptide;
    class peptides;
    class protease;

    class  ADPROTSHARED_EXPORT digestedPeptides  {
    public:
        digestedPeptides();
        digestedPeptides( const digestedPeptides& );
        digestedPeptides( const protein&, const protease& );

        const protein& protein() const;
        const protease& protease() const;
        const peptides& peptides() const;
		digestedPeptides& operator << ( const adprot::peptide& );

    private:
        std::shared_ptr< adprot::protein > protein_;
        std::shared_ptr< adprot::protease > protease_;
        std::shared_ptr< adprot::peptides > peptides_;
    };

}

#endif // DIGESTEDPEPTIDES_HPP
