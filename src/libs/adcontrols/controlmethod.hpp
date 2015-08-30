// This is a -*- C++ -*- header.
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

#include "adcontrols_global.h"
#include "idaudit.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class idAudit;

    namespace controlmethod {

        class ADCONTROLSSHARED_EXPORT MethodItem {
        public:
            MethodItem();
            MethodItem( const MethodItem& );
            MethodItem( const std::string& model, uint32_t unitnumber = 0, uint32_t funcid = 0 );
            
            // An analytical instrument is consisted from serveral independent modules
            // such as autosampler, solvent delivery system, 2 units of (same models of) UV detector
            // and so on.  Each module can be indentified by a pair of modelname and unitnumber that 
            // is counted from zero (zero is default number).

            const std::string& modelname() const;
            void setModelname( const char * );
            
            uint32_t unitnumber() const;
            void unitnumber( uint32_t );

            bool isInitialCondition() const;
            void isInitialCondition( bool );

            const double& time() const;
            void time( const double& );

            uint32_t funcid() const;
            void funcid( uint32_t );

            const std::string& itemLabel() const; // short description for Table UI
            void setItemLabel( const char * );

            const char * data() const;            // serialized data
            size_t size() const;
            void data( const char * data, size_t size );

            const std::string& description() const;
            void setDescription( const char * );

            template< typename T > static bool set( MethodItem& mi, const T& t
                                                    , std::function<bool( std::ostream&, const T& )> serialize = T::archive) {
                std::ostringstream o;
                if ( serialize( o, t ) ) {
                    mi.data( o.str().data(), o.str().size() );
                    return true;
                }
                return false;
            }

            template< typename T > static bool get( const MethodItem& mi, T& t
                                                    , std::function<bool( std::istream&, T& )> deserialize = T::restore ) {
                std::istringstream i( std::string( mi.data(), mi.size() ) );
                return deserialize( i, t );
            }
            
        private:

#if defined _MSC_VER
# pragma warning( push )
# pragma warning( disable: 4251 )
#endif
            std::string modelname_;
            uint32_t unitnumber_;
            bool isInitialCondition_;
            double time_;
            uint32_t funcid_;              // MethodFunc
            std::string label_;
            std::string data_;             // serialized data
            std::string description_;      // utf8
#if defined _MSC_VER
# pragma warning( pop )
#endif            
        private:
            friend class boost::serialization::access;
            template<class Archive>
                void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP(modelname_)
                    & BOOST_SERIALIZATION_NVP(unitnumber_)
                    & BOOST_SERIALIZATION_NVP(isInitialCondition_)
                    & BOOST_SERIALIZATION_NVP(time_)
                    & BOOST_SERIALIZATION_NVP(funcid_)
                    & BOOST_SERIALIZATION_NVP(label_)
                    & BOOST_SERIALIZATION_NVP(data_)
                    ;
                if ( version >= 2 )
                    ar & BOOST_SERIALIZATION_NVP( description_ );
            }
        };

    };

    ////////////////////////////////////////////////

    class ADCONTROLSSHARED_EXPORT ControlMethod {
    public:
        ~ControlMethod();
        ControlMethod();
        ControlMethod( const ControlMethod& );
        ControlMethod& operator = ( const ControlMethod& );

        static const wchar_t * dataClass() { return L"adcontrols::ControlMethod"; }
        typedef size_t size_type;
        typedef std::vector< controlmethod::MethodItem >::iterator iterator;
        typedef std::vector< controlmethod::MethodItem >::const_iterator const_iterator;
        
        const char * description() const;
        void setDescription( const char * );

        const char * subject() const;
        void setSubject( const char * );

        size_type size() const;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        iterator erase( iterator pos );
        iterator erase( iterator first, iterator last );
        iterator insert( const controlmethod::MethodItem& );
        void push_back( const controlmethod::MethodItem& );
        const idAudit& ident() const;
        void sort();
        void clear();

        static bool archive( std::ostream&, const ControlMethod& );
        static bool restore( std::istream&, ControlMethod& );
        static bool xml_archive( std::wostream&, const ControlMethod& );
        static bool xml_restore( std::wistream&, ControlMethod& );

    private:
        class impl;
#if defined _MSC_VER
# pragma warning( push )
# pragma warning( disable: 4251 )
#endif
        std::unique_ptr< impl > impl_;
#if defined _MSC_VER
# pragma warning( pop )
#endif
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int );
    };

    typedef std::shared_ptr<ControlMethod> ControlMethodPtr;

}

BOOST_CLASS_VERSION( adcontrols::ControlMethod, 2 )
BOOST_CLASS_VERSION( adcontrols::controlmethod::MethodItem, 2 )

