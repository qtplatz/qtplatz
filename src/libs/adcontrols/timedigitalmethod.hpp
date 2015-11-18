// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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
#include <boost/serialization/version.hpp>

#include "threshold_method.hpp"
#include "threshold_action.hpp"
#include <vector>
#include <iostream>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

#if defined _MSC_VER
    ADCONTROLSSHARED_TEMPLATE_EXPORT template class ADCONTROLSSHARED_EXPORT std::vector < threshold_method > ;
#endif

    template<typename T> class TimeDigitalMethod_archive;

    class ADCONTROLSSHARED_EXPORT TimeDigitalMethod {
    public:
        ~TimeDigitalMethod(void);
        TimeDigitalMethod(void);
		TimeDigitalMethod( const TimeDigitalMethod& );

        static const char * modelClass() { return "TimeDigitalMethod"; }

        std::vector< threshold_method >& thresholds();
        const std::vector< threshold_method >& thresholds() const;

        const threshold_method& threshold( size_t ch ) const;
        void setThreshold( size_t ch, const threshold_method& );

        threshold_action& action();
        const threshold_action& action() const;

        static bool archive( std::ostream&, const TimeDigitalMethod& );
        static bool restore( std::istream&, TimeDigitalMethod& );
        static bool xml_archive( std::wostream&, const TimeDigitalMethod& );
        static bool xml_restore( std::wistream&, TimeDigitalMethod& );
        
    private:
        std::vector< threshold_method > thresholds_;
        threshold_action action_;
        
        friend class TimeDigitalMethod_archive < TimeDigitalMethod > ;
        friend class TimeDigitalMethod_archive < const TimeDigitalMethod > ;
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
	};
    
}

BOOST_CLASS_VERSION( adcontrols::TimeDigitalMethod, 0 )

