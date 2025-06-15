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

#include "mzmlchromatogram.hpp"
#include "binarydataarray.hpp"
#include <pugixml.hpp>

namespace mzml {

    class mzMLChromatogram::impl {
    public:
        binaryDataArray prime_;
        binaryDataArray secondi_;
        pugi::xml_node node_;

    public:
        impl() {}
        impl( binaryDataArray prime
              , binaryDataArray secondi
              , pugi::xml_node node ) : prime_( prime )
                                      , secondi_( secondi )
                                      , node_( node ) {
        }
    };


    mzMLChromatogram::~mzMLChromatogram()
    {
    }

    mzMLChromatogram::mzMLChromatogram() : impl_( std::make_unique< impl >() )
    {
    }

    mzMLChromatogram::mzMLChromatogram( binaryDataArray prime
                                        , binaryDataArray secondi
                                        , pugi::xml_node node ) : impl_( std::make_unique< impl >( prime, secondi, node ) )
    {
    }

    size_t
    mzMLChromatogram::length() const
    {
        return impl_->prime_.length(); // held in base class
    }

    const pugi::xml_node&
    mzMLChromatogram::node()
    {
        return node(); // held in base class
    }
}
