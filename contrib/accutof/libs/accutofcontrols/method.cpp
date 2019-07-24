/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "method.hpp"
#include "accutofcontrols_global.hpp"
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/tofprotocol.hpp>
#include <compiler/boost/workaround.hpp>


namespace accutofcontrols {
    constexpr boost::uuids::uuid method::__clsid;
}

using namespace accutofcontrols;

///////////////////

method::~method()
{
}

method::method()
{
}

method::method( const method& t ) : json_( t.json_ )
{
}

//static
bool
method::archive( std::ostream&, const method& )
{
    assert( 0 ); // todo
    return true;
}

//static
bool
method::restore( std::istream&, method& )
{
    assert( 0 ); // todo
    return true;
}
