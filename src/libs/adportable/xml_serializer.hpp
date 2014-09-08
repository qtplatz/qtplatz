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

#include <workaround/boost/archive/xml_woarchive.hpp>
#include <workaround/boost/archive/xml_wiarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

namespace adportable {

    namespace xml {

        template<typename, typename>
        struct has_archive;

        template<typename T, typename Ret, typename... Args>
        struct has_archive<T, Ret(Args...)> {
            template<typename U, U> struct SFINAE;
            template<typename U> static std::true_type test( SFINAE<Ret(*)(Args...), &U::xml_archive>* ); // check 'static' mem_fun
            template<typename U> static std::false_type test(...);
            static const bool value = decltype(test<T>(0))::value;
        };

        template<typename, typename>
        struct has_restore;

        template<typename T, typename Ret, typename... Args>
        struct has_restore<T, Ret(Args...)> {
            template<typename U, U> struct SFINAE;
            template<typename U> static std::true_type test( SFINAE<Ret(*)(Args...), &U::xml_restore>* ); // check 'static' mem_fun
            template<typename U> static std::false_type test(...);
            static const bool value = decltype(test<T>(0))::value;
        };

        template<class T> class archive_functor {
            std::wostream& os_;
        public:
            archive_functor( std::wostream& os ) : os_( os ){}
            void operator & ( const T& t ) { T::xml_archive( os_, t ); }        
        };

        template<class T> class restore_functor {
            std::wistream& is_;
        public:
            restore_functor( std::wistream& is ) : is_( is ){}
            void operator & ( T& t ) { T::xml_restore( is_, t ); }
        };

        template<bool b> struct nvp_functor {
            template<class T> inline const boost::serialization::nvp< T > operator()( T& data ) const {
                return boost::serialization::nvp<T>( "data", data );
            }
        };
        template<> struct nvp_functor<true> {
            template<typename T> const T& operator()( const T& data ) const {
                return data;
            }
            template<typename T> T& operator()( T& data ) const {
                return data;
            }
        };

        //------------------------------------------
        template<bool, class Archiver, class Functor> struct IF { typedef Archiver type; };
        template<class Archiver, class Functor> struct IF<true, Archiver, Functor> { typedef Functor type; };

        //-------------------- serialize ------------------------
        template<class Archiver = boost::archive::xml_woarchive> class serialize {
        public:
            template<class T> bool operator()( const T& data, std::wostream& output ) const {

                typename IF< has_archive<T, bool( std::wostream&, const T& )>::value, Archiver, archive_functor<T> >::type ar( output );

                ar & nvp_functor< has_archive<T, bool( std::wostream&, const T& )>::value >()( data );

                output.flush();
                return true;
            }

            template<class T> bool operator()( const T& data, std::wstring& output ) const {

                boost::iostreams::back_insert_device< std::wstring > inserter( output );
                boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );

                typename IF< has_archive<T, bool( std::wostream&, const T& )>::value, Archiver, restore_functor<T> >::type ar( output );

                ar & nvp_functor< has_archive<T, bool( std::wostream&, const T& )>::value >()( data );

                device.flush();
                return true;
            }
        };

        //-------------------- deserialize ------------------------
        template<class Archiver = boost::archive::xml_wiarchive> class deserialize {
        public:

            template<class T> bool operator()( T& data, std::wistream& strm ) {

                typename IF<has_restore<T,bool(std::wistream&,T&)>::value, Archiver, restore_functor<T> >::type ar( strm );

                ar & nvp_functor< has_restore<T, bool( std::wistream&, T& )>::value >()( data ) ;

                return true;
            }

        };

    } // namespace xml
    //----------------------------------------
    
}

