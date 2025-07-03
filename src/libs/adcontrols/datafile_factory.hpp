// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <filesystem>

namespace adcontrols {

    class datafile;
    enum access_mode { read_access = 1, write_access = 2 };

    class ADCONTROLSSHARED_EXPORT datafile_factory {
    public:
        datafile_factory(void) {}
        virtual ~datafile_factory(void) {}
		virtual const char * mimeTypes() const = 0;
        virtual const wchar_t * name() const = 0;

        [[deprecated("use std::filesystem::path interface")]]
        virtual bool access( const wchar_t * filename, access_mode = read_access ) const = 0;
        virtual bool access( const std::filesystem::path& filename, access_mode = read_access ) const = 0;

        [[deprecated("use std::filesystem::path interface")]]
        virtual datafile * open( const wchar_t * filename, bool readonly = false ) const = 0;
        virtual datafile * open( const std::filesystem::path& filename, bool readonly = false ) const = 0;
        virtual void close( datafile * ) = 0;
    private:

    };

}
