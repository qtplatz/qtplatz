// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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
#include <boost/smart_ptr.hpp>

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
    class ProcessMethod;
    class ElementalCompositionCollection;
    class MSCalibrateResult;
	class PeakResult;
}

namespace adutils {

    typedef boost::shared_ptr< adcontrols::MassSpectrum > MassSpectrumPtr;
    typedef boost::shared_ptr< adcontrols::Chromatogram > ChromatogramPtr;
    typedef boost::shared_ptr< adcontrols::ProcessMethod > ProcessMethodPtr;
    typedef boost::shared_ptr< adcontrols::ElementalCompositionCollection > ElementalCompositionCollectionPtr;
    typedef boost::shared_ptr< adcontrols::MSCalibrateResult > MSCalibrateResultPtr;
    typedef boost::shared_ptr< adcontrols::PeakResult > PeakResultPtr;

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
#if defined __GNUC__
            return std::string( a.type().name() ) == typeid( T ).name();
#else
            return a.type() == typeid( T );
#endif            
        }

    private:
        value_type datum_;
    };

}

