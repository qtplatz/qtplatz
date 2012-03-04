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
#include <string>

namespace adcontrols {

    class Visitor;
    class DataInterpreter;
    
    class ADCONTROLSSHARED_EXPORT MassSpectrometer {
    public:
        MassSpectrometer(void) {}
        virtual ~MassSpectrometer(void) {}
        
        class ScanLaw {
        public:
            virtual double getMass( double secs, int type ) const = 0;
            virtual double getTime( double mass, int type ) const = 0;
            virtual double getMass( double secs, double fLength ) const = 0;
            virtual double getTime( double mass, double fLength ) const = 0;
        };
        typedef MassSpectrometer * (*factory_type)(void);
        
        virtual void accept( Visitor& ) = 0;
        virtual factory_type factory() = 0;
        virtual const wchar_t * name() const = 0;
        virtual const ScanLaw& getScanLaw() const = 0;
        virtual const DataInterpreter& getDataInterpreter() const = 0;
        
        static const MassSpectrometer& get( const std::wstring& modelname );
    };
    
}

