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

#include "adprocessor_global.hpp"
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <memory>
#include <optional>

namespace boost {
    namespace uuids {
        class uuid;
    }
}

namespace adcontrols {
    class Chromatogram;
    class descriptions;
}

namespace adprocessor {

    class ADPROCESSORSHARED_EXPORT generator_property;

    class generator_property {
    public:
        ~generator_property();
        generator_property();
        generator_property( const generator_property& );
        generator_property& operator = ( const generator_property& );
        generator_property( const adcontrols::Chromatogram& );
        std::string generator() const;
        std::optional< std::string > formula() const;
        std::string adduct() const;
        double mass() const;
        double mass_width() const;
        const std::string& data_reader() const;
        int protocol() const;
        void set_dataSource( std::pair< std::string, boost::uuids::uuid >&& );
        std::pair< std::string, boost::uuids::uuid > dataSource() const;

        std::tuple< double, std::string, std::string > get() const;
        const boost::json::value& value() const;

        friend ADPROCESSORSHARED_EXPORT void
        tag_invoke( boost::json::value_from_tag, boost::json::value&, const generator_property& );

        friend ADPROCESSORSHARED_EXPORT generator_property
        tag_invoke( boost::json::value_to_tag< generator_property >&, const boost::json::value& );

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
