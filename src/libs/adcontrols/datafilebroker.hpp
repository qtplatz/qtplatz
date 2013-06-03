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

#include "adcontrols_global.h"
#include "visitor.hpp"
#include <string>

namespace adcontrols {

    class datafile_factory;
    class datafile;

    class ADCONTROLSSHARED_EXPORT datafileBroker : public Visitor {
    protected:
        ~datafileBroker();
        datafileBroker();
    public:
        static bool register_factory( datafile_factory *, const std::wstring& name );
        static datafile_factory* find( const std::wstring& name );
        //
        static datafile * create( const std::wstring& filename );
        static datafile * open( const std::wstring& filename, bool readonly = false );
    };
    
}

