// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include <boost/any.hpp>
#include <boost/variant.hpp>
#include <adportable/is_type.hpp>
#include <memory>

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
    class ProcessMethod;
    class ElementalCompositionCollection;
    class MSCalibrateResult;
	class PeakResult;
}

namespace adutils {

    typedef std::shared_ptr< adcontrols::MassSpectrum > MassSpectrumPtr;
    typedef std::shared_ptr< adcontrols::Chromatogram > ChromatogramPtr;
    typedef std::shared_ptr< adcontrols::ProcessMethod > ProcessMethodPtr;
    typedef std::shared_ptr< adcontrols::ElementalCompositionCollection > ElementalCompositionCollectionPtr;
    typedef std::shared_ptr< adcontrols::MSCalibrateResult > MSCalibrateResultPtr;
    typedef std::shared_ptr< adcontrols::PeakResult > PeakResultPtr;

    class ProcessedData {
    public:
        ProcessedData();

        class Nothing { 
        public:
            Nothing() {}
        };

        typedef boost::variant< Nothing
                              , MassSpectrumPtr
                              , ChromatogramPtr
                              , ProcessMethodPtr
                              , ElementalCompositionCollectionPtr 
                              , MSCalibrateResultPtr 
							  , PeakResultPtr 
                              > value_type;

        static value_type toVariant( boost::any& );

        template<class T> static bool is_type( boost::any& a ) {
			return adportable::a_type< T >::is_a( a );
        }

    private:
        value_type datum_;
    };

}

