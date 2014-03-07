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
#include <boost/variant.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <vector>
#include <memory>
#include <string>
#include <compiler/disable_dll_interface.h>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    namespace controlmethod {

        class ADCONTROLSSHARED_EXPORT MethodItem {
        public:
            MethodItem();
            MethodItem( const MethodItem& );
            MethodItem( const std::string& model, uint32_t unitnumber, uint32_t funcid = 0 );

            // An analytical instrument is consisted from serveral independent modules
            // such as autosampler, solvent delivery system, 2 units of (same models of) UV detector
            // and so on.  Each module can be indentified by a pair of modelname and unitnumber that 
            // is counted from zero (zero is default number).

            const std::string& modelname() const;
            void modelname( const std::string& );
            
            uint32_t unitnumber() const;
            void unitnumber( uint32_t );

            bool isInitialCondition() const;
            void isInitialCondition( bool );

            const double& time() const;
            void time( const double& );

            uint32_t funcid() const;
            void funcid( uint32_t );

            const std::string& itemLabel() const; // short description for Table UI
            void itemLabel( const std::string& );

            const char * data() const;            // serialized data
            size_t size() const;
            void data( const char * data, size_t size );

        private:
            std::string modelname_;
            uint32_t unitnumber_;
            bool isInitialCondition_;
            double time_;
            uint32_t funcid_;              // MethodFunc
            std::string label_;
            std::string data_;             // serialized data

        private:
            friend class boost::serialization::access;

            template<class Archive>
                void serialize( Archive& ar, const unsigned int ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP(modelname_)
                    & BOOST_SERIALIZATION_NVP(unitnumber_)
                    & BOOST_SERIALIZATION_NVP(isInitialCondition_)
                    & BOOST_SERIALIZATION_NVP(time_)
                    & BOOST_SERIALIZATION_NVP(funcid_)
                    & BOOST_SERIALIZATION_NVP(label_)
                    & BOOST_SERIALIZATION_NVP(data_)
                    ;
            }
        };

    };

    ////////////////////////////////////////////////

    class ADCONTROLSSHARED_EXPORT ControlMethod {
    public:
        ~ControlMethod();
        ControlMethod();
        ControlMethod( const ControlMethod& );
        // ControlMethod& operator = ( const ControlMethod& );

        static const wchar_t * dataClass() { return L"adcontrols::ControlMethod"; }
        typedef size_t size_type;
        typedef std::vector< controlmethod::MethodItem >::iterator iterator;
        typedef std::vector< controlmethod::MethodItem >::const_iterator const_iterator;

        size_type size() const;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        iterator erase( iterator pos );
        iterator erase( iterator first, iterator last );
        iterator insert( const controlmethod::MethodItem& );

    private:
        friend class boost::serialization::access;

        std::string subject_;
        std::string description_;
        std::vector< controlmethod::MethodItem > items_;

        template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP(subject_)
                & BOOST_SERIALIZATION_NVP(description_)
                & BOOST_SERIALIZATION_NVP(items_);
        }
    };

    typedef std::shared_ptr<ControlMethod> ControlMethodPtr;

}

BOOST_CLASS_VERSION( adcontrols::ControlMethod, 1 )
BOOST_CLASS_VERSION( adcontrols::controlmethod::MethodItem, 1 )

