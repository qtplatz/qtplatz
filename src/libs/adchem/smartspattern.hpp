/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "adchem_global.h"

#include <boost/shared_ptr.hpp>

namespace OpenBabel { class OBSmartsPattern; }

namespace adchem {

    class Mol;

	class ADCHEMSHARED_EXPORT SmartsPattern {
		boost::shared_ptr< OpenBabel::OBSmartsPattern > obSmartsPattern_;
	public:
        ~SmartsPattern();
        SmartsPattern();

        bool init( const char * pattern );
        bool match( const Mol&, bool single = false );

        const OpenBabel::OBSmartsPattern * get() const;
        void assign( const OpenBabel::OBSmartsPattern& );
	};

}


