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

#include "adcontrols_global.h"
#include "visitor.h"
#include <string>

namespace adcontrols {

    class MassSpectrometer;

    class ADCONTROLSSHARED_EXPORT MassSpectrometerBroker : public Visitor {
    protected:
        MassSpectrometerBroker(void);
        ~MassSpectrometerBroker(void);
    public:
        typedef MassSpectrometer * (*factory_type)(void);
        
        static bool register_library( const std::wstring& sharedlib );
        static bool register_factory( factory_type, const std::wstring& name );
        static factory_type find( const std::wstring& name );
    };
    
}
