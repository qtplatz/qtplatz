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

#include "folder.hpp"
#include "file.hpp"
#include <adportable/debug.hpp>

using namespace py_module;

folder::folder()
{
    ADDEBUG() << "folder::ctor copy default";
}

folder::folder( const folder& t ) : folder_( t.folder_ )
{
}

folder::folder( const adfs::folder& t ) : folder_(t)
{
}

folder::~folder()
{
}

uint64_t
folder::rowid() const
{
    return folder_.rowid();
}

std::wstring
folder::name() const
{
    return folder_.name();
}

std::wstring
folder::id() const
{
    return folder_.id();
}

boost::python::dict
folder::attributes() const
{
    boost::python::dict dict;

    for ( const auto& a: folder_ )
        dict[ a.first ] = a.second;
    return dict;
}

// std::vector< std::shared_ptr< folder > >
boost::python::list
folder::folders() const
{
    auto v = folder_.folders();

    boost::python::list list;
    for ( auto sub: v )
        list.append( folder( sub ) );

    return list;
}

boost::python::list
folder::files() const
{
    boost::python::list list;
    for ( auto sub: folder_.files() ) {
        ADDEBUG() << "sub file -- rowid=" << sub.rowid() << ", name=" << sub.name() << ", id=" << sub.id();
        list.append( file( sub ) );
    }

    return list;
}
