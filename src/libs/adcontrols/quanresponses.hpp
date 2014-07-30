/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANRESPONSES_HPP
#define QUANRESPONSES_HPP

#include "adcontrols_global.h"
#include "quanresponse.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

#if defined _MSC_VER
    template class ADCONTROLSSHARED_EXPORT std::vector < QuanResponse > ;
#endif

    class ADCONTROLSSHARED_EXPORT QuanResponses  {
    public:
        QuanResponses();
        QuanResponses( const QuanResponses& t ) : values_( t.values_ ) {}

        QuanResponses& operator << (const QuanResponse& t) { values_.push_back( t ); return *this; }

        size_t size() const { return values_.size(); }
        void clear() { values_.clear(); }
        std::vector< QuanResponse >::iterator begin() { return values_.begin(); }
        std::vector< QuanResponse >::iterator end() { return values_.end(); }
        std::vector< QuanResponse >::const_iterator begin() const { return values_.begin(); }
        std::vector< QuanResponse >::const_iterator end() const { return values_.end(); }

    private:
        std::vector< QuanResponse > values_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( values_ )
                ;
        }
    };
}

#endif // QUANRESPONSES_HPP
