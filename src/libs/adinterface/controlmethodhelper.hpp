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

#ifndef CONTROLMETHODHELPER_HPP
#define CONTROLMETHODHELPER_HPP

#include <adinterface/controlmethodC.h>

namespace adcontrols {
    namespace ControlMethod { class Method; class MethodItem; }
}

namespace adinterface {

    class ControlMethodHelper {
    public:
        template<class T> static void replace_or_add( ::ControlMethod::Method& m
                                               , const std::string& device
                                               , bool isInitialCondition = true
                                               , double time = (-1)
                                               , uint32_t unitNumber = 1 ) {
            replace_or_add( m, device, T::modelClass(), T::itemLabel(), isInitialCondition, time, unitNumber );
        }

        template<class T> static const ::ControlMethod::MethodLine * find( const ::ControlMethod::Method& m, uint32_t unitNumber = 1 ) {
            return find( m, T::modelClass(), T::itemLabel(), unitNumber );
        }

        template<class T> static ::ControlMethod::MethodLine * find( ::ControlMethod::Method& m, uint32_t unitNumber = 1 ) {
            return find( m, T::modelClass(), T::itemLabel(), unitNumber );
        }

        static void replace_or_add( ::ControlMethod::Method& m
                                    , const std::string& device
                                    , const char * modelname
                                    , const char * itemname
                                    , bool isInitialCondition = true
                                    , double time = (-1)
                                    , uint32_t unitNumber = 1 );
        
        static const ::ControlMethod::MethodLine * find( const ControlMethod::Method&
                                                        , const char * modelname
                                                        , const char * itemname
                                                        , uint32_t unitNumber = 1 );

        static ::ControlMethod::MethodLine * find( ControlMethod::Method&
                                                        , const char * modelname
                                                        , const char * itemname
                                                        , uint32_t unitNumber = 1 );

        static void copy( ::ControlMethod::Method&, const adcontrols::ControlMethod::Method& );

        static void copy( adcontrols::ControlMethod::Method&, const ::ControlMethod::Method& );

    private:
        static void append( ::ControlMethod::Method& m
                            , const std::string& device
                            , const char * modelname
                            , const char * itemlabel
                            , bool isInitialCondition = true
                            , double time = (-1)
                            , uint32_t unitNumber = 1 );
    };

}

#endif // CONTROLMETHODHELPER_HPP
