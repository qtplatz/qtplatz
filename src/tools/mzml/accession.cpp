/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "accession.hpp"
#include <algorithm>
#include <array>
#include <numeric>
#include <tuple>

namespace mzml {

    const std::array< std::tuple< std::string, Accession, std::string >, MS_MAX > accession_list = {{
            {	"MS:1000016",	MS_1000016,	"scan time"	}
            ,{	"MS:1000026",	MS_1000026,	"detector type"	}
            ,{	"MS:1000040",	MS_1000040,	"m/z"	}
            ,{	"MS:1000042",	MS_1000042,	"intensity"	}
            ,{	"MS:1000045",	MS_1000045,	"collision energy"	}
            ,{	"MS:1000073",	MS_1000073,	"electrospray ionization"	}
            ,{	"MS:1000081",	MS_1000081,	"quadrupole"	}
            ,{	"MS:1000093",	MS_1000093,	"increasing m/z scan"	}
            ,{	"MS:1000095",	MS_1000095,	"linear"	}
            ,{	"MS:1000111",	MS_1000111,	"electron multiplier tube"	}
            ,{	"MS:1000124",	MS_1000124,	"Shimadzu instrument model"	}
            ,{	"MS:1000127",	MS_1000127,	"centroid mass spectrum"	}
            ,{	"MS:1000128",	MS_1000128,	"profile spectrum"	}
            ,{	"MS:1000129",	MS_1000129,	"negative scan"	}
            ,{	"MS:1000130",	MS_1000130,	"positive scan"	}
            ,{	"MS:1000133",	MS_1000133,	"collision-induced dissociation"	}
            ,{	"MS:1000235",	MS_1000235,	"total ion current chromatogram"	}
            ,{	"MS:1000264",	MS_1000264,	"ion trap"	}
            ,{	"MS:1000285",	MS_1000285,	"total ion current"	}
            ,{	"MS:1000294",	MS_1000294,	"mass spectrum"	}
            ,{	"MS:1000398",	MS_1000398,	"nanoelectrospray"	}
            ,{	"MS:1000498",	MS_1000498,	"full scan"	}
            ,{	"MS:1000500",	MS_1000500,	"scan m/z upper limit"	}
            ,{	"MS:1000501",	MS_1000501,	"scan m/z lower limit"	}
            ,{	"MS:1000504",	MS_1000504,	"base peak m/z"	}
            ,{	"MS:1000505",	MS_1000505,	"base peak intensity"	}
            ,{	"MS:1000511",	MS_1000511,	"ms level"	}
            ,{	"MS:1000512",	MS_1000512,	"filter string"	}
            ,{	"MS:1000514",	MS_1000514,	"m/z array"	}
            ,{	"MS:1000515",	MS_1000515,	"intensity array"	}
            ,{	"MS:1000521",	MS_1000521,	"32-bit float"	}
            ,{	"MS:1000523",	MS_1000523,	"64-bit float"	}
            ,{	"MS:1000527",	MS_1000527,	"highest m/z value"	}
            ,{	"MS:1000528",	MS_1000528,	"lowest m/z value"	}
            ,{	"MS:1000529",	MS_1000529,	"instrument serial number"	}
            ,{	"MS:1000544",	MS_1000544,	"Conversion to mzML"	}
            ,{	"MS:1000554",	MS_1000554,	"LCQ Deca"	}
            ,{	"MS:1000563",	MS_1000563,	"Xcalibur RAW file"	}
            ,{	"MS:1000569",	MS_1000569,	"SHA-1"	}
            ,{	"MS:1000574",	MS_1000574,	"compressed"	}
            ,{	"MS:1000576",	MS_1000576,	"no compression"	}
            ,{	"MS:1000579",	MS_1000579,	"MS1 spectrum"	}
            ,{	"MS:1000580",	MS_1000580,	"MSn spectrum"	}
            ,{	"MS:1000583",	MS_1000583,	"SRM spectrum"	}
            ,{	"MS:1000595",	MS_1000595,	"time array"	}
            ,{	"MS:1000628",	MS_1000628,	"basepeak chromatogram"	}
            ,{	"MS:1000744",	MS_1000744,	"selected ion m/z"	}
            ,{	"MS:1000776",	MS_1000776,	"scan number only nativeID format"	}
            ,{	"MS:1000795",	MS_1000795,	"no combination"	}
            ,{	"MS:1000810",	MS_1000810,	"mass chromatogram"	}
            ,{	"MS:1000827",	MS_1000827,	"isolation window target m/z"	}
            ,{	"MS:1001557",	MS_1001557,	"Shimadzu Corporation software"	}
        }};

    accession::accession()
    {
    }

    accession::accession( const accession& t ) : names_( t.names_ )
    {
    }

    std::optional< Accession >
    accession::assign( const std::string& a, const std::string& name )
    {
        auto it = std::find_if(mzml::accession_list.begin()
                               , mzml::accession_list.end(), [&](auto item){ return std::get<0>(item) == a; } );
        if ( it != mzml::accession_list.end() ) {
            names_[ std::get<1>(*it) ] = name;
            return std::get<1>(*it);
        }
        return {};
    }

    std::optional< std::string >
    accession::name( Accession a ) const
    {
        auto it = names_.find( a );
        if ( it != names_.end() )
            return it->second;
        return {};
    }

    std::string
    accession::toString() const
    {
        return std::accumulate( names_.begin()
                                , names_.end()
                                , std::string{},
                                [](const auto& a, const auto& b){
                                    return a.empty() ? b.second : a + "," + b.second; } );
    }

    bool
    accession::is_mz() const
    {
        return names_.find( mzml::MS_1000514 ) != names_.end();
    }

    bool
    accession::is_intensity() const
    {
        return names_.find( mzml::MS_1000515 ) != names_.end();
    }

    bool
    accession::is_base64() const
    {
        return names_.find( mzml::MS_1000576 ) != names_.end();
    }

    bool
    accession::is_compressed() const
    {
        return names_.find( mzml::MS_1000574 ) != names_.end();
    }

    bool
    accession::is_64bit() const
    {
        return names_.find( mzml::MS_1000523 ) != names_.end();
    }

    bool
    accession::is_32bit() const
    {
        return names_.find( mzml::MS_1000521 ) != names_.end();
    }

    bool
    accession::empty() const
    {
        return names_.empty();
    }
}
