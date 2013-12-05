/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include <adcontrols/msproperty.hpp>
#include <adcontrols/massspectrum.hpp>
#include <boost/variant.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <adportable/debug.hpp>

namespace batchproc {

    class import_profile : boost::noncopyable {
    public:
        adcontrols::MSProperty prop_;
        adcontrols::MS_POLARITY polarity_;
        std::vector< float > intensities_;
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & prop_
                & polarity_
                & intensities_;
        }
    };

    class import_continuum_massarray {
    public:
        int32_t pos_;
        std::vector< double > masses_;
        import_continuum_massarray() : pos_(0) {}
        import_continuum_massarray( const import_continuum_massarray& t ) : pos_( t.pos_ )
                                                                          , masses_( t.masses_ ) {
        }
    private:
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & pos_;
            adportable::debug( __FILE__, __LINE__ ) << "import_continuum_massarray serialize pos=" << pos_;
            ar & masses_;
            adportable::debug( __FILE__, __LINE__ ) << "import_continuum_massarray serialize masses=" << masses_.size();
        }
    };

}

