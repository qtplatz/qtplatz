// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <streambuf>
#include <memory>
#include <adfs/file.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

namespace adfs {

    class file;

    // namespace detail { 
    //     class cpio : public std::basic_streambuf<char> {
	// 		std::unique_ptr< char [] > p_;
    //         size_t size_;
    //         size_t count_;
    //         bool resize( size_t );
    //     public:
    //         cpio() : size_(0), count_(0) {}
    //         cpio( size_t size, char * p = 0 );

    //         inline size_t size() const { return count_; }
    //         inline const char * get() const { return p_.get(); }
    //         inline char * get() { return p_.get(); }
    //     protected:
    //         virtual std::streamsize xsputn( const char * s, std::streamsize num );
    //         virtual std::basic_streambuf<char>::int_type overflow ( int_type c );
    //         virtual std::basic_streambuf<char>::int_type underflow();
    //     };
    // };

    /*template<class data_type> */ 
    class cpio {
    public:
#if 0
        template<class T> static bool serialize( const T& t, detail::cpio& obuf ) {
            std::ostream os( &obuf );
            return T::archive( os, t );
        }

        template<class T> static bool deserialize( T& t, detail::cpio& ibuf ) {
            std::istream is( &ibuf );
            return T::restore( is, t );
        }
#endif
        template<class T> static bool serialize( const T& t, std::string& ar ) {
            boost::iostreams::back_insert_device< std::string > inserter( ar );
            boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
            return T::archive( device, t );
        }

        template<class T> static bool deserialize( T& t, const char * data, size_t length ) {
            boost::iostreams::basic_array_source< char > device( data, length );
            boost::iostreams::stream< boost::iostreams::basic_array_source< char > > st( device );
            return T::restore( st, t );
        }

    };

}
