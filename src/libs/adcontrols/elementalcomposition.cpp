// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "elementalcomposition.hpp"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

using namespace adcontrols;

ElementalComposition::ElementalComposition()
{
}

namespace adcontrols {

    template<> void
    ElementalComposition::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        (void)version;
        // ar << boost::serialization::make_nvp( "ElementalComposition", pImpl_ );
    }

    template<> void
    ElementalComposition::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        (void)version;
        // ar >> boost::serialization::make_nvp("ElementalComposition", pImpl_);
    }

    template<> void
    ElementalComposition::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        (void)version;
        // ar << boost::serialization::make_nvp("ElementalComposition", pImpl_);
    }

    template<> void
    ElementalComposition::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        (void)version;
        // ar >> boost::serialization::make_nvp("ElementalComposition", pImpl_);
    }

}
