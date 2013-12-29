// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#pragma once

#include "adplugin_global.h"
///#include <boost/any.hpp>
namespace boost { class any; }

namespace adportable {
    class Configuration;
}

namespace adplugin {

    class ADPLUGINSHARED_EXPORT LifeCycle {
    public:
        virtual ~LifeCycle();
        LifeCycle();
        virtual void OnCreate( const adportable::Configuration& ) = 0;
        virtual void OnInitialUpdate() = 0;
        virtual void OnFinalClose() = 0;
        virtual void onUpdate( boost::any& ) {}
        virtual bool getContents( boost::any& ) const { return false; }
        virtual bool setContents( boost::any& ) { return false; }
        virtual void * query_interface_workaround( const char * typenam ) { (void)typenam; return 0; }

        template<typename T> T* query_interface() {
            T* p = dynamic_cast<T*>(this);
            if ( !p )
                p = reinterpret_cast<T*>( query_interface_workaround( typeid(T).name() ) );
            return p;
        }
		
    protected:
		inline bool isScoped() const { return scope_flag_; }

        bool scope_flag_;

        struct ADPLUGINSHARED_EXPORT scoped_lock {
            LifeCycle& x_;
            scoped_lock( LifeCycle& );
            ~scoped_lock();
        };
    };

}


