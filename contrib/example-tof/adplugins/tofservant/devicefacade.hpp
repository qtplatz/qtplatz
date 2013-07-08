// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#ifndef DEVICEFACADE_HPP
#define DEVICEFACADE_HPP

#include <adportable/configuration.hpp>
#include <acewrapper/mutex.hpp>
#include <boost/variant.hpp>
#include <tofinterface/tofC.h>
#include "avgr_emu.hpp"

namespace TOF { struct ControlMethod; }

namespace tofservant {
    
    class toftask;
    
    typedef boost::variant< std::shared_ptr< avgr_emu > > device_variant;

    class DeviceFacade {
        friend class toftask;
		DeviceFacade();
        DeviceFacade( const DeviceFacade& ); // non copyable
	public:
        ~DeviceFacade();

        bool setConfiguration( const char * xml );
        bool setControlMethod( const TOF::ControlMethod&, const char * hint );
        bool initialize();
        bool terminate();
        bool async_apply_method( const TOF::ControlMethod& );

        template<class T> T* get() {
            for ( device_variant& v: vec_ )
                try { return ( boost::get<std::shared_ptr<T> >( v )).get(); } catch ( std::bad_cast& ) {}
            return 0;
        }

    private:
        bool initialized_;
        std::string configXml_;
        std::vector< device_variant > vec_;
    };

}

#endif // DEVICEFACADE_HPP
