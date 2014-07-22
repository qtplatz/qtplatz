/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANDATAWRITER_HPP
#define QUANDATAWRITER_HPP

#include <adfs/filesystem.hpp>
#include <string>

namespace adcontrols { class MassSpectrum; }

namespace quan {

    class QuanDataWriter  {
    public:
        ~QuanDataWriter();
        QuanDataWriter( const std::wstring& path );

        bool open();
        bool write( const adcontrols::MassSpectrum& ms, const std::wstring& tittle, std::wstring& id );

    private:
        std::wstring path_;
        adfs::filesystem fs_;
    };

}

#endif // QUANDATAWRITER_HPP
