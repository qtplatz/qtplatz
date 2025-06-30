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

#pragma once

#include <string>
#include <pugixml.hpp>
#include <boost/json/fwd.hpp>

namespace mzml {

    enum ion_polarity_type: unsigned int;

    class scan_protocol {
        int ms_level_;  // 1 := MS1 (or SIM), // 2 := Q3 scan (or SRM)
        double precursor_mz_;
        double collision_energy_;
        ion_polarity_type polarity_;
        double scan_window_lower_limit_;
        double scan_window_upper_limit_;
    public:
        scan_protocol();
        scan_protocol( const scan_protocol& );
        scan_protocol( pugi::xml_node );
        int ms_level() const;
        double precursor_mz() const;
        double collision_energy() const;
        ion_polarity_type polarity() const;
        double scan_window_lower_limit() const;
        double scan_window_upper_limit() const;

        template< uint32_t Resolution = 100 >
        std::tuple< int                     // ms_level
                    , ion_polarity_type
                    , int                   // precorsor mz * Resolution
                    , int                   // collision energy * 10
                    , int                   // scan_window_lower_limit
                    , int                   // scan_window_upper_limit
                    > protocol_key() const {
            return { ms_level_
                     , polarity_
                     , int(precursor_mz_ * Resolution + 0.5)
                     , int(collision_energy_ * 10 + 05)
                     , int(scan_window_lower_limit_ * Resolution + 0.5)
                     , int(scan_window_upper_limit_ * Resolution + 0.5) };
        }
        friend std::ostream& operator<<(std::ostream&, const scan_protocol&);
        friend scan_protocol tag_invoke( const boost::json::value_to_tag< scan_protocol >&, const boost::json::value&);
    };

    using scan_protocol_key_t = decltype(std::declval<mzml::scan_protocol>().protocol_key<>());

    struct protocol_key_hash {
        template<typename T>
        inline void hash_combine(std::size_t& seed, const T& val) const {
            seed ^= std::hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        std::size_t operator()(const scan_protocol_key_t& key) const;
    };

    struct protocol_key_equal {
        bool operator()(const scan_protocol_key_t& lhs, const scan_protocol_key_t& rhs) const {
            return lhs == rhs;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////

    using scan_id =
        std::tuple< int  // index
                    , std::string // id
                    , double // scan start time
                    , scan_protocol
                    >;

    enum scan_id_enum { enum_scan_index, enum_scan_id, enum_scan_start_time, enum_scan_protocol };

    struct scan_id_accessor {
        const scan_id& _;
        inline int scan_index() const                     { return std::get<0>( _ ); }
        inline const std::string& get_scan_id() const     { return get<1>( _ ); }
        inline double scan_start_time() const             { return std::get< 2 >( _ ); }
        inline const scan_protocol& get_scan_protocol() const { return std::get< 3 >( _ ); }
    };

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const scan_protocol& );

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const scan_id& );

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const scan_protocol_key_t& );

    class scan_identifier {
    public:
        scan_identifier();
        scan_id operator()( const pugi::xml_node& spectrum_node ) const;
    };

} // namespace
