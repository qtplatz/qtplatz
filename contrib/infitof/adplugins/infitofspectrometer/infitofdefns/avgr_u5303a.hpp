/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include <u5303a/digitizer.hpp>
#include <acqrscontrols/u5303a/metadata.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>

namespace infitof {

    namespace u5303a {
        
        class Descriptors {
        public:
            Descriptors() 
            {}
            Descriptors(const acqrscontrols::u5303a::device_method& m, const acqrscontrols::u5303a::metadata& meta) : method_(m), meta_( meta )
            {}

            acqrscontrols::u5303a::device_method method_;
            acqrscontrols::u5303a::metadata meta_;
            
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize( Archive& ar, const unsigned int version );
        };
        
    };
    
}

BOOST_CLASS_VERSION( infitof::u5303a::Descriptors, 1 )
