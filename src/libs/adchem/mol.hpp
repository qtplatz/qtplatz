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

#ifndef MOL_HPP
#define MOL_HPP
#include "adchem_global.h"
#include "attributes.hpp"

#include <boost/smart_ptr.hpp>


namespace OpenBabel { class OBMol; }

namespace adchem {

	class ADCHEMSHARED_EXPORT Mol {
	public:
        ~Mol();
		Mol();
        Mol( const Mol& );
        Mol& operator = ( const Mol& );
        
		void obmol( OpenBabel::OBMol& );
		const OpenBabel::OBMol * obmol() const;

		double getExactMass( bool implicitH = true ) const;
        const char * getFormula() const;
		void setAttribute( const char * key, const char * value );

        attributes get_attributes() const;

		operator OpenBabel::OBMol& ();
		operator const OpenBabel::OBMol& () const;

	private:
		boost::shared_ptr< OpenBabel::OBMol > obmol_;
		bool dirty_;
		double exactmass_;
        adchem::attributes attrs_;
        std::string formula_;
        void update();
	};

}

#endif // MOL_HPP
