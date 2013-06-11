/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef CONVERSION_HPP
#define CONVERSION_HPP

#include "adchem_global.h"
#include "string.hpp"
#include <boost/shared_ptr.hpp>

namespace OpenBabel { 
    class OBFormat;
    class OBConversion;
}

namespace adchem {

	class Mol;
    
	class ADCHEMSHARED_EXPORT Conversion {
        std::string filename_;
        size_t nread_;
        boost::shared_ptr< OpenBabel::OBConversion > obconversion_;
	public:
        virtual ~Conversion();
        Conversion();
        Conversion( const Conversion& );

        unsigned long long tellg() const;
        void informat( const OpenBabel::OBFormat * );
        void open( const char * filename );
        bool read( Mol& );

        static string toSMILES( const Mol& );
        static string toSVG( const Mol& );
        // static size_t toSVG( const OpenBabel::OBMol&, char *&svg );
        // static size_t toSMILES( const OpenBabel::OBMol&, char *& smiles );
	};

}

#endif // CONVERSION_HPP
