/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef ADFSIO_HPP
#define ADFSIO_HPP

#include <adfs/adfs.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adportable/serializer.hpp>

namespace adutils {



    template<class T> class adfsio {

    public:
        static bool read( adfs::folder& folder, T& t, const std::wstring& file_name ) {
            std::vector< adfs::file > files = folder.files();
            auto it = std::find_if( files.begin(), files.end(), [=]( const adfs::file& f ){ return f.name() == file_name; });
            if ( it != files.end() ) {
                std::unique_ptr< char [] > device( new char [ it->size() ] );
                if ( it->read( it->size(), device.get() ) == it->size() )
                    return adportable::serializer<T>::deserialize( t, device.get(), it->size() );
            }
            return false;
        }

        static bool write( adfs::folder& folder, const T& t, const std::wstring& file_name ) {
            adfs::file file = folder.addFile( file_name );
            std::string device;
            if ( adportable::serializer< T >::serialize( t, device ) )
                return file.write( device.size(), device.data() ) == device.size();
            return false;
        }
    };

}

#endif // ADFSIO_HPP
