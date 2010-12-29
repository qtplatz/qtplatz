// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <boost/any.hpp>
#include <boost/variant.hpp>
#include <boost/smart_ptr.hpp>

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
    class ProcessMethod;
    class ElementalCompositionCollection;
}

namespace adutils {

    typedef boost::shared_ptr< adcontrols::MassSpectrum > MassSpectrumPtr;
    typedef boost::shared_ptr< adcontrols::Chromatogram > ChromatogramPtr;
    typedef boost::shared_ptr< adcontrols::ProcessMethod > ProcessMethodPtr;
    typedef boost::shared_ptr< adcontrols::ElementalCompositionCollection > ElementalCompositionCollectionPtr;

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
                              > value_type;

        static value_type toVariant( boost::any& );

    private:
        value_type datum_;
    };

}

