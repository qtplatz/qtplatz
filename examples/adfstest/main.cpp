// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include <adfs/adfs.h>
#include <adfs/adsqlite.h>
#include <iostream>

struct column_print : public boost::static_visitor<void> {
    template<typename T> void operator()( T& t ) const {
        std::cout << t;
    }
};

template<> void column_print::operator ()( adfs::blob& ) const
{
}

template<> void column_print::operator ()( std::wstring& t ) const
{
    std::wcout << t;
}

template<> void column_print::operator ()( adfs::null& ) const
{
}

template<> void column_print::operator ()( adfs::error& ) const
{
}

int
main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);

    adfs::sqlite db;
    
    db.open( L"disk.adfs" );

    adfs::stmt sql( db );

    sql.prepare( "select * from zTable" );

    while ( sql.step() == adfs::sqlite_row ) {
        std::size_t size = sql.column_count();
        std::cout << "\ncolumn_count=" << size << std::endl;
        for ( std::size_t i = 0; i < size; ++i ) {
            std::cout << "type:" << sql.column_type(i) << "'";
            boost::apply_visitor( column_print(), sql.column_value(i) );
            std::cout << "'\t";
        }
    };

/*
    adfs::storage stg;
    stg.create( "disk.adfs" );
    stg.close();
*/
}
