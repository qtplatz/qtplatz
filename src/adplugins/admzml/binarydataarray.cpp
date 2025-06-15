// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "binarydataarray.hpp"
#include "accession.hpp"
#include <adportable/base64.hpp>

namespace mzml {

    binaryDataArray::binaryDataArray( size_t length
                                      , mzml::accession&& ac
                                      , std::string&& decoded ) : encodedLength_( length )
                                                                     , ac_( ac )
                                                                     , decoded_( decoded )
    {
    }

    binaryDataArray::binaryDataArray( const binaryDataArray& t ) : encodedLength_( t.encodedLength_ )
                                                                 , ac_( t.ac_ )
                                                                 , decoded_( t.decoded_ )
    {
    }

    binaryDataArray::operator bool () const
    {
        return not decoded_.empty();
    }

    const accession&
    binaryDataArray::accession() const
    {
        return ac_;
    }
    size_t
    binaryDataArray::size() const { return decoded_.size(); }

    size_t
    binaryDataArray::length() const {
            if ( ac_.is_32bit() ) {
                return size() / sizeof(float);
            } else if ( ac_.is_64bit() ) {
                return size() / sizeof(double);
            }
            return size();
        }

    data_ptr
    binaryDataArray::data() const {
        if ( ac_.is_32bit() )
            return reinterpret_cast< const float * >( decoded_.data() );
        if ( ac_.is_64bit() )
            return reinterpret_cast< const double * >( decoded_.data() );
        return {};
    }

    binaryDataArray
    binaryDataArray::make_instance( const pugi::xml_node& node ) {
        size_t length = node.attribute( "encodedLength" ).as_uint();
        mzml::accession ac(node);

        std::string decoded;
        if ( auto binary = node.select_node( "binary" ) ) {
            std::string_view encoded = binary.node().child_value();
            if ( encoded.size() == length )
                decoded  = base64_decode( encoded );
        }
        return { length, std::move( ac ), std::move( decoded ) };
    }
} // namespace
