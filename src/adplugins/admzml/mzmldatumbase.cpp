// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
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

#include "mzmldatumbase.hpp"

namespace mzml {

    class mzMLDatumBase::impl {
    public:
        pugi::xml_node node_;
        impl() {}
        impl( const impl& t ) : node_( t.node_ ) {}
        impl( pugi::xml_node node ) : node_( node ) {}
    };
}


using namespace mzml;

mzMLDatumBase::~mzMLDatumBase()
{
}

mzMLDatumBase::mzMLDatumBase() : impl_( std::make_unique< impl >() )
{
}

mzMLDatumBase::mzMLDatumBase( const mzMLDatumBase& t ) : impl_( std::make_unique< impl >(*t.impl_) )
{
}

mzMLDatumBase::mzMLDatumBase( pugi::xml_node node ) : impl_( std::make_unique< impl >( node ) )
{
}

std::string_view
mzMLDatumBase::id() const
{
    return impl_->node_.attribute( "id" ).value();
}

size_t
mzMLDatumBase::index() const
{
    return impl_->node_.attribute( "index" ).as_uint();
}
