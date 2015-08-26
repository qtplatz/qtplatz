/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#pragma once

#include "adicontroller_global.hpp"

#include <cstdint>
#include <vector>
#include <string>

namespace adicontroller {
    namespace ControlMethod {

        struct MethodLine;

#if defined _MSC_VER
        ADICONTROLLERSHARED_TEMPLATE_EXPORT template class ADICONTROLLERSHARED_EXPORT std::vector < MethodLine > ;
        ADICONTROLLERSHARED_TEMPLATE_EXPORT template class ADICONTROLLERSHARED_EXPORT std::vector < int8_t > ;
#endif

        typedef std::vector< MethodLine > method_sequence;
        typedef std::vector< int8_t > octet_array;
        
        enum eDeviceCategory {
            device_unknown
            , device_sampler               // autosampler or anything prepare sample (trigger injection)
            , device_gas_chromatograph     // just GC w/ or w/o injector
            , device_liquid_chromatograph  // degasser, pump, gradienter, oven (sync to an injection)
            , device_detector              // LC/GC detectors including MS
        };

        // corresponding to adcontrols::controlmethod::MethodItem

        struct ADICONTROLLERSHARED_EXPORT MethodLine {

            MethodLine();
            MethodLine( const MethodLine& );

            const char * modelname() const;
            const void setModelname( const std::string& );

            const char * description() const;
            const void setDescription( const std::string& );

            const char * itemLable() const;
            const void setItemLabel( const std::string& );

            const octet_array& xdata() const;
            octet_array& xdata();

        private:

#if defined _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif
            std::string   modelname_;
            std::string   description_;
            uint32_t      unitnumber_;           // uint32_t
            bool          isInitialCondition_;
            double        time_;
            uint32_t      funcid_;               // uint32_t device dependent
            std::string   itemlabel_;         // short description for funcid
            octet_array   xdata_;             // device dependent serialized data
#if defined _MSC_VER
# pragma warning(pop)
#endif
        };
        
        
        struct ADICONTROLLERSHARED_EXPORT Method {

            Method();
            Method( const Method& );

            const char * subject() const;
            void setSubject( const std::string& );

            const char * description() const;
            void setDescription( const std::string& );

            const method_sequence& lines() const;
            method_sequence& lines();

        private:
#if defined _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif
            std::string subject_;
            std::string description_;

#if defined _MSC_VER
#pragma warning(pop)
#endif
            method_sequence lines_;
        };
        
    }
}

