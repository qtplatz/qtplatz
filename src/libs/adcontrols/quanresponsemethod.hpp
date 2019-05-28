/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adcontrols_global.h"
#include "idaudit.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT QuanResponseMethod;
    enum idToleranceMethod : int;
    enum idFindAlgorithm : int;

    class QuanResponseMethod {

    public:
        ~QuanResponseMethod();
        QuanResponseMethod();
        QuanResponseMethod( const QuanResponseMethod& );

        enum idWidthMethod { idWidthDaltons, idWidthPpm };
        enum idDataSelect  { idAverage, idLargest };
        enum idIntensity   { idArea, idCentroid };

        idWidthMethod widthMethod() const;
        void setWidthMethod( idWidthMethod );
        double width( idWidthMethod = idWidthDaltons ) const;
        void setWidth( double, idWidthMethod = idWidthDaltons );

        idDataSelect dataSelectionMethod() const;
        void setDataSelectionMethod( idDataSelect );

        idFindAlgorithm findAlgorithm() const;
        void setFindAlgorithm( idFindAlgorithm );

        idIntensity intensityMethod() const;
        void setIntensityMethod( idIntensity );

        std::string toJson() const;
        void fromJson( const std::string& );

    private:
        idIntensity intensityMethod_;
        idFindAlgorithm findAlgo_;
        idWidthMethod widthMethod_;
        double widthPpm_;
        double widthDaltons_;
        idDataSelect dataSelect_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( intensityMethod_ )
                & BOOST_SERIALIZATION_NVP( findAlgo_ )
                & BOOST_SERIALIZATION_NVP( widthMethod_ )
                & BOOST_SERIALIZATION_NVP( dataSelect_ )
                & BOOST_SERIALIZATION_NVP( widthDaltons_ )
                & BOOST_SERIALIZATION_NVP( widthPpm_ )
                ;
        };
    };

}

BOOST_CLASS_VERSION( adcontrols::QuanResponseMethod, 1 )
