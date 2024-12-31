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

#include "ra.hpp"
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/debug.hpp>

namespace adcontrols {
    namespace peakd {

        class RA::impl {
        public:
            std::tuple< double, std::string > dataSource_;
            std::vector< RA::value_type > values_;
            impl() {}
            impl(const impl& t ) : dataSource_( t.dataSource_ )
                                 , values_( t.values_ ) {
            }
        };


        RA::RA() : impl_( new impl{} )
        {
        }

        RA::RA( const RA& t ) : impl_( new impl{ *t.impl_ } )
        {
        }
        RA::~RA()
        {
            delete impl_;
        }

        void
        RA::setDataSource( double tR, const std::string& file )
        {
            impl_->dataSource_ = std::make_tuple( tR, file );
        }

        std::tuple< double, std::string >
        RA::dataSource() const
        {
            return impl_->dataSource_;
        }


        std::vector< RA::value_type >&
        RA::values()
        {
            return impl_->values_;
        }

        const std::vector< RA::value_type >&
        RA::values() const
        {
            return impl_->values_;
        }

        void
        tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const RA& t )
        {
            jv = {
                { "dataSource", boost::json::value_from( t.impl_->dataSource_) }
                , { "values", boost::json::value_from( t.impl_->values_ ) }
            };
        }

        RA
        tag_invoke( const boost::json::value_to_tag< RA >&, const boost::json::value& jv )
        {
            using namespace adportable::json;
            RA t;
            if ( jv.kind() == boost::json::kind::object ) {
                extract( jv.as_object(), t.impl_->dataSource_, "dataSource" );
                extract( jv.as_object(), t.impl_->values_, "values" );
            }
            return t;
        }


    } // <------------------------
}
