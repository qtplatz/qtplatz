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
#include <adportable/debug.hpp>

using namespace py_module;

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
file::files() const
{
    // auto v = file_.files();

    // ADDEBUG() << "file::files rowid=" << rowid() << "\tdb=" << (void*)(&file_.db()) << "\t" << file_.name() << ", " << v.size() << " subfiles found";
    // for ( const auto& sub: v )
    //     ADDEBUG() << "sub file -- rowid=" << sub.rowid() << ", name=" << sub.name() << ", id=" << sub.id();

    boost::python::list list;
    // for ( auto sub: v )
    //     list.append( file( sub ) );

    return list;
}
