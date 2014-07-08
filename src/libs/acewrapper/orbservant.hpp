// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#ifndef ORBSERVANT_H
#define ORBSERVANT_H

#include <string>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_deprecated.h>

#include <tao/ORB.h>
#include <tao/PortableServer/PortableServer.h>
#include <compiler/diagnostic_pop.h>

class TAO_ORB_Manager;

namespace acewrapper {

    template<class T> class ORBServant {
    public:
        ~ORBServant() { 
        }
	
        ORBServant( ) {
        }

        void initialize( CORBA::ORB_ptr orb, PortableServer::POA_ptr poa, PortableServer::POAManager_ptr mgr ) {
            orb_ = orb; // already _duplicate'ed in tao
            poa_ = poa;
            poa_manager_ = mgr;
        }
	
        inline void activate() {
            PortableServer::ObjectId_var id = poa_->activate_object ( &impl_ );
            CORBA::Object_var obj = poa_->id_to_reference (id.in ());
            CORBA::String_var str = orb_->object_to_string (obj.in ());
            id_ = str;
        }
	
        void deactivate() { 
            PortableServer::ObjectId_var object_id = poa_->servant_to_id ( &impl_ );
            poa_->deactivate_object ( object_id.in () );
        }
	
        inline operator T* () { return &impl_; }
        inline T& impl()      { return impl_; }
        inline CORBA::ORB_ptr orb() { return CORBA::ORB::_duplicate( orb_.in() ); }
        inline PortableServer::POA_ptr poa() { return PortableServer::POA::_duplicate( poa_.in() ); }
        inline PortableServer::POAManager_ptr poa_manager() { 
            return PortableServer::POAManager::_duplicate( poa_manager_.in() ); }
	
        inline operator typename T::_stub_ptr_type () { return impl_._this(); }
        inline const std::string& ior() const { return id_; }
	
    private:
        CORBA::ORB_var orb_;
        PortableServer::POA_var poa_;
        PortableServer::POAManager_var poa_manager_;
        std::string id_;
        T impl_;
    };
    
}


#endif // ORBSERVANT_H
