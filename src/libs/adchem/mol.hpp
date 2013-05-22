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

#include <boost/smart_ptr.hpp>
#include <string>
#include <vector>

namespace OpenBabel { class OBMol; }

namespace adchem {

	class Mol {
		boost::shared_ptr< OpenBabel::OBMol > mol_;
	public:
        ~Mol();
		Mol();
        Mol( const Mol& );
		Mol( const OpenBabel::OBMol& );
		Mol& operator = ( const OpenBabel::OBMol& );
        Mol& operator = ( const Mol& );

		double getExactMass( bool implicitH = true ) const;
        std::string getFormula() const;
		void setAttribute( const std::string& key, const std::string& value );
		operator OpenBabel::OBMol& ();
		operator const OpenBabel::OBMol& () const;

		static double GetExactMass( const OpenBabel::OBMol&, bool implicitH = true );
		static std::string GetFormula( const OpenBabel::OBMol& );
		static void SetAttribute( OpenBabel::OBMol&, const std::string& key, const std::string& value );
		static std::vector< std::pair< std::string, std::string > > attributes( const OpenBabel::OBMol& mol
			, const std::vector< std::string >& excludes = std::vector< std::string >() );
	private:
		bool dirty_;
		double exactmass_;
	};

}

#endif // MOL_HPP
