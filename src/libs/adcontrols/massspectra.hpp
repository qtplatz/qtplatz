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

#include "adcontrols_global.h"

#include <string>
#include <memory>
#include <boost/serialization/vector.hpp>
#include <compiler/disable_dll_interface.h>

namespace adcontrols {

    class MassSpectrum;

    class ADCONTROLSSHARED_EXPORT MassSpectra {
    public:
		static const wchar_t * dataClass() { return L"MassSpectra"; }

        typedef std::shared_ptr< MassSpectrum > value_type;

        void push_back( const MassSpectrum& );
        void push_back( value_type& );

        size_t size() const;
        MassSpectrum& operator [] ( size_t fcn );
        const MassSpectrum& operator [] ( size_t fcn ) const;

    private:
        std::vector< value_type > vec_;

	    friend class boost::serialization::access;
	    template<class Archive> void serialize(Archive& ar, const unsigned int version) {
            (void)version;
            ar & vec_;
        }
    };    

    typedef std::shared_ptr<MassSpectra> MassSpectraPtr;
   
}


