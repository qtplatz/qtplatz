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

#include <string>
#include <map>
#include <optional>
#include <pugixml.hpp>

namespace mzml {
	enum Accession {
        MS_1000016
        , MS_1000026
        , MS_1000040
        , MS_1000042
        , MS_1000045
        , MS_1000073
        , MS_1000081
        , MS_1000093
        , MS_1000095
        , MS_1000111
        , MS_1000124
        , MS_1000127
        , MS_1000128
        , MS_1000129
        , MS_1000130
        , MS_1000133
        , MS_1000235
        , MS_1000264
        , MS_1000285
        , MS_1000294
        , MS_1000398
        , MS_1000498
        , MS_1000500
        , MS_1000501
        , MS_1000504
        , MS_1000505
        , MS_1000511
        , MS_1000512
        , MS_1000514
        , MS_1000515
        , MS_1000521
        , MS_1000523
        , MS_1000527
        , MS_1000528
        , MS_1000529
        , MS_1000544
        , MS_1000554
        , MS_1000563
        , MS_1000569
        , MS_1000574
        , MS_1000576
        , MS_1000579
        , MS_1000580
        , MS_1000583
        , MS_1000595
        , MS_1000628
        , MS_1000744
        , MS_1000776
        , MS_1000795
        , MS_1000810
        , MS_1000827
        , MS_1001557
        , MS_MAX
	};

    class accession {
        using attribute_t = std::map< std::string, std::string >;

        std::map< std::string, attribute_t > accession_;
        std::optional< std::string > name( const attribute_t& ) const;
    public:
        accession();
        accession( const accession& );
        accession( const pugi::xml_node& parent_node );

        operator bool () const;
        bool empty() const;
        std::optional< Accession > assign( const std::string& a, const std::string& name );
        std::optional< std::string > name( Accession ) const;
        std::optional< std::string > name( const std::string& ) const;
        std::string toString() const;
        bool is_mz() const;
        bool is_intensity() const;
        bool is_base64() const;
        bool is_compressed() const;
        bool is_64bit() const;
        bool is_32bit() const;
    };

} // namespace
