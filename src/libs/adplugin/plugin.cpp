/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: info@ms-cheminfo.com
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

#include "plugin.hpp"
#include "orbfactory.hpp"
#include "orbservant.hpp"
#include <adportable/debug.hpp>

using namespace adplugin;

plugin::~plugin()
{
    adportable::debug(__FILE__, __LINE__) << "##### plugin dtor called #####";
}

plugin::plugin() : ref_count_( 1 )
{
}

plugin::plugin( const plugin& t ) : clsid_( t.clsid_ )
                                  , ref_count_( t.ref_count_ )
{
    adportable::debug(__FILE__, __LINE__) << "==== plugin copy called #####";
}

void
plugin::add_ref()
{
    ++ref_count_;
    adportable::debug(__FILE__, __LINE__) << "===== plugin add_ref(" << ref_count_ << ") #####";
}

void 
plugin::release()
{
    adportable::debug(__FILE__, __LINE__) << "===== plugin release(" << ref_count_ << ") #####";
    if ( ref_count_ ) {
        if ( --ref_count_ == 0 )
            delete this;
     }
}
