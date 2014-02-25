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

#ifndef FSIO2_HPP
#define FSIO2_HPP

#include <string>
#include <boost/filesystem.hpp>

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
    class ProcessMethod;
    class ElementalCompositionCollection;
    class MSCalibrateResult;
	class PeakResult;
	class datafile;
}

namespace adinterface {
    class Method;
}

namespace adfs { class filesystem; class folder; class file; }
namespace portfolio { class Portfolio; class Folium; }

namespace adutils {
	
	class fsio2 {
    public:
        fsio2();

        static bool saveContents( adfs::filesystem&, const std::wstring&, const portfolio::Portfolio&, const adcontrols::datafile& );
        static bool appendOnFile( const std::wstring& file, const portfolio::Folium&, const adcontrols::datafile& );
    };

}

#endif // FSIO2_HPP
