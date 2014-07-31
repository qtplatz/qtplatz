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

#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/file.hpp>
#include <string>
#include <mutex>
#include <functional>

namespace adcontrols { class MassSpectrum; class ProcessMethod; class QuanSequence; class QuanSample; class QuanCompounds; class QuanMethod; }

namespace quan {

    class QuanDataWriter  {
        QuanDataWriter( const QuanDataWriter& ) = delete;
    public:
        ~QuanDataWriter();
        QuanDataWriter( const std::wstring& path );

        bool open();
        adfs::file write( const adcontrols::MassSpectrum& ms, const std::wstring& tittle );
        adfs::file write( const adcontrols::ProcessMethod& );
        adfs::file write( const adcontrols::QuanSequence& );
        adfs::file write( const adcontrols::QuanSample& );

        bool drop_table();
        bool create_table();

        bool insert_table( const adcontrols::QuanMethod& );
        bool insert_table( const adcontrols::QuanSequence& );
        bool insert_table( const adcontrols::QuanCompounds& );
        bool insert_table( const adcontrols::QuanSample& );

        template< class T> adfs::file attach( adfs::file& file, const T& t, const std::wstring& name ) {
            auto afile = file.addAttachment( adfs::create_uuid() );
            afile.dataClass( T::dataClass() );
            if ( !name.empty() )
                afile.setAttribute( L"name", name );
            if ( adfs::cpio< T >::save( t, afile ) )
                afile.commit();
            return afile;
        }

    private:
        std::mutex mutex_;
        std::wstring path_;
        adfs::filesystem fs_;
        std::string uuidQuanCompound_;
    };

}

#endif // QUANDATAWRITER_HPP
