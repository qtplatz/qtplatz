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

    class ADCONTROLSSHARED_EXPORT ControlMethod {
    public:
        ~ControlMethod();
        ControlMethod();
        ControlMethod( const ControlMethod& );

        static const wchar_t * dataClass() { return L"adcontrols::ControlMethod"; }

        typedef size_t size_type;

        class ADCONTROLSSHARED_EXPORT MethodLine {
        public:
            MethodLine();
            MethodLine( const MethodLine& );

            const std::wstring& modelname() const;
            void modelname( const std::wstring& );
            uint32_t unitnumber() const;
            void unitnumber( uint32_t );
            bool isInitialCondition() const;
            void isInitialCondition( bool );
            const std::pair< uint32_t, uint32_t >& time() const;
            void time( const std::pair< uint32_t, uint32_t >& );
            uint32_t funcid() const;
            void funcid( uint32_t );
            const uint8_t * xdata() const;
            size_t xsize() const;

        private:
            std::wstring modelname_;
            uint32_t unitnumber_;
            bool isInitialCondition_;
            std::pair< uint32_t /* sec */, uint32_t /* usec */ > time_;
            uint32_t funcid_;         // MethodFunc
            std::vector< uint8_t > xdata_; // serialized data

        private:
            template<class Archive>
                void serialize( Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                (void)version;
                ar & BOOST_SERIALIZATION_NVP(modelname_);
                ar & BOOST_SERIALIZATION_NVP(unitnumber_);
                ar & BOOST_SERIALIZATION_NVP(isInitialCondition_);
                ar & BOOST_SERIALIZATION_NVP(time_);
                ar & BOOST_SERIALIZATION_NVP(funcid_);
                ar & BOOST_SERIALIZATION_NVP(xdata_);
            }
        };

        size_type size() const;

    private:
        std::wstring subject_;
        std::wstring description_;
        std::vector< MethodLine > lines_;

        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            (void)version;
            ar & BOOST_SERIALIZATION_NVP(subject_);
            ar & BOOST_SERIALIZATION_NVP(description_);
            ar & BOOST_SERIALIZATION_NVP(lines_);
        }
    };

    typedef std::shared_ptr<ControlMethod> ControlMethodPtr;

}


