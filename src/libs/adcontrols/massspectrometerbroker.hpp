// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@MS-Cheminformatics.com
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
//#include "visitor.hpp"
#include <string>
#include <vector>

namespace adcontrols {

    class MassSpectrometer;
	class massspectrometer_factory;

    class ADCONTROLSSHARED_EXPORT massSpectrometerBroker { //: public Visitor {
    protected:
        massSpectrometerBroker(void);
        ~massSpectrometerBroker(void);
    public:
        static bool register_factory( massspectrometer_factory*, const std::wstring& name );
        static massspectrometer_factory* find( const std::wstring& name );
        static std::vector< std::wstring > names();
    };
    
}
