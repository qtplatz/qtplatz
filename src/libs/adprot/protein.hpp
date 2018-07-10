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

#ifndef PROTEIN_HPP
#define PROTEIN_HPP

#include "adprot_global.hpp"
#include <string>

namespace adprot {

    class ADPROTSHARED_EXPORT protein {
    public:
        protein();
        protein( const protein& );
        protein( const std::string& name, const std::string& sequence );
        
        const std::string& name() const;
        const std::string& sequence() const;
        void name( const std::string& );
        void sequence( const std::string& );

    private:
        std::string name_;
        std::string sequence_;
    };

}

#endif // PROTEIN_HPP
