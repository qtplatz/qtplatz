/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC
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

#include "file.hpp"
#include "../py_adcontrols/peakresult.hpp"
#include <adcontrols/typelist.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/serializer.hpp>
#include <adportable/debug.hpp>
#include <boost/exception/all.hpp>

using namespace py_module;

namespace py_module {

    struct data_class_visitor : public boost::static_visitor< adcontrols::data_class_t > {

        std::pair< const char *, size_t > data_;
        data_class_visitor( const char * data, size_t size ) : data_{ data, size } {};

        template< typename T >
        adcontrols::data_class_t operator()( std::shared_ptr<T> t ) const {
            try {
                if ( adfs::cpio::deserialize(*t, data_.first, data_.second ) ) {
                    return t;
                    // return boost::python::object( t );
                }
            } catch ( ... ) {
                ADDEBUG() << boost::current_exception_diagnostic_information();
            }
            return {};
        }
    };

    struct data_class_py_wrapper : public boost::static_visitor< boost::python::object > {

        template< typename T >
        boost::python::object operator()( std::shared_ptr<T> t ) const {
            return boost::python::object( t );
        }

        // boost::python::object operator()( std::shared_ptr< adcontrols::PeakResult> t ) const {
        //     return boost::python::object( py_module::PeakResult( t ) );
        // }

    };
#if 0
    template<>
    boost::python::object data_class_py_wrapper::operator()( std::shared_ptr< adcontrols::PeakResult> t ) const {
        return boost::python::object( py_module::PeakResult( t ) );
    }
#endif
}

file::file()
{
}

file::file( const file& t ) : file_( t.file_ )
{
}

file::file( const adfs::file& t ) : file_(t)
{
}

file::~file()
{
}

uint64_t
file::rowid() const
{
    return file_.rowid();
}

std::wstring
file::name() const
{
    return file_.name();
}

std::wstring
file::id() const
{
    return file_.id();
}

boost::python::dict
file::attributes() const
{
    boost::python::dict dict;

    for ( const auto& a: file_ )
        dict[ a.first ] = a.second;
    return dict;
}

boost::python::list
file::attachments() const
{
    auto v = file_.attachments();

    boost::python::list list;
    for ( auto sub: v )
        list.append( file( sub ) );

    return list;
}

// ref
// https://stackoverflow.com/questions/29119086/how-to-convert-c-objects-to-boostpythonobject

boost::python::object
file::body() const
{
    const auto dataClass = file_.attribute( L"dataType" );
    adfs::stmt sql( file_.db() );

    sql.prepare( "SELECT data FROM file WHERE fileid = (SELECT rowid FROM directory WHERE name = ?)" );
    sql.bind( 1 ) = file_.attribute( L"dataId" );

    if ( sql.step() == adfs::sqlite_row ) {
        auto blob = sql.get_column_value< adfs::blob >( 0 );
        if ( blob.size() ) {
            auto body = adcontrols::data_class_tlist()( dataClass );
            auto obj = boost::apply_visitor( data_class_visitor( reinterpret_cast< const char * >( blob.data() ), blob.size() ), body );
            if ( obj != adcontrols::data_class_t{} ) {
                return boost::apply_visitor( data_class_py_wrapper(), obj );
            }
        }
    }
    return {};
}
