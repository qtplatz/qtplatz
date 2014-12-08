/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <string>
#include <boost/any.hpp>
#include <adinterface/controlmethodC.h>
#include <adinterface/controlmethodhelper.hpp>
#include <adportable/is_type.hpp>

namespace adinterface {

    template<class IM, class Serializer>
    class ControlMethodAccessorT {
        std::wstring instId_;
        unsigned int unitNumber_;
    public:
        ControlMethodAccessorT( const std::wstring& instId
                                , unsigned int unitnumber = 0 ) : instId_( instId )
                                                                , unitNumber_( unitnumber ) {
        }

        bool setMethod( ::ControlMethod::Method& dst, const IM& im ) const {
            std::string device;
            if ( Serializer::serialize( im, device ) ) {
                ::ControlMethod::MethodLine * line = ControlMethodHelper::findFirst( dst, instId_, unitNumber_ );
                if ( line == 0 ) {
                    ::ControlMethod::MethodLine& t = ControlMethodHelper::add( dst, instId_, unitNumber_ );
                    line = &t;
                }
                line->xdata.length( CORBA::ULong( device.size() ) );
                std::copy( device.begin(), device.end(), reinterpret_cast< char *>(line->xdata.get_buffer()) );
                return true;
            }
            return false;
        }

        bool setMethod( boost::any& a, const IM& im ) const {

            if ( adportable::a_type< ::ControlMethod::Method >::is_pointer( a ) ) {
                auto m = boost::any_cast< ::ControlMethod::Method * >( a );
                return setMethod( *m, im );
            }
            return false;
        }

        bool getMethod( IM& im, const ::ControlMethod::Method& source ) {
            const ::ControlMethod::MethodLine * line = ControlMethodHelper::findFirst( source, instId_, unitNumber_ );
            if ( line && line->xdata.length() > 0 )
                return Serializer::deserialize( im, reinterpret_cast< const char * >(line->xdata.get_buffer()), line->xdata.length() );
            return false;
        }

        bool getMethod( IM& im, boost::any& a ) {
            if ( const ::ControlMethod::Method * method = 
                 adportable::a_type< ::ControlMethod::Method >::is_pointer( a ) ? 
                 boost::any_cast< ::ControlMethod::Method * >( a ) :
                 &(boost::any_cast< const ::ControlMethod::Method& >( a )) ) {
                
                return getMethod( im, *method );                
            }
            return false;
        }
    };

}

