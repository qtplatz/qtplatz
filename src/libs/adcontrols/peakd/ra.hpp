// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
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

#include "adcontrols_global.h"
#include <string>
#include <tuple>
#include <memory>
#include <vector>
#include <boost/json/value_from.hpp>

namespace adcontrols {

    namespace peakd {

        class ADCONTROLSSHARED_EXPORT RA;

        class RA {
        public:
            using value_type = std::tuple< double, double, std::string, std::tuple<int,double,std::string > >;

            ~RA();
            RA();
            RA( const RA& );

            void setDataSource( double tR, const std::string& file );
            std::tuple< double, std::string > dataSource() const;

            std::vector< value_type >& values();
            const std::vector< value_type >& values() const;

            std::string ident() const;
            static std::string ident( const RA& ra );

        private:
            class impl;
            impl * impl_;

            friend ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const RA& );
            friend ADCONTROLSSHARED_EXPORT RA tag_invoke( const boost::json::value_to_tag< RA >&, const boost::json::value& jv );
        };

        ADCONTROLSSHARED_EXPORT
        void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const RA& );

        ADCONTROLSSHARED_EXPORT
        RA tag_invoke( const boost::json::value_to_tag< RA >&, const boost::json::value& jv );

    } // namespace peakd

}
