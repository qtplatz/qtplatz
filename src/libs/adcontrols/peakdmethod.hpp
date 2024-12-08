// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC
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
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/variant.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <adcontrols/timeutil.hpp>
#include <vector>

namespace adcontrols {

    class PeakDMethod {
    public:
        ~PeakDMethod(void);
        PeakDMethod(void);
		PeakDMethod(const PeakDMethod &);

		PeakDMethod & operator = ( const PeakDMethod & rhs );

	private:

        double minimumHeight_;
        double minimumArea_;
        double minimumWidth_;
        double doubleWidthTime_;
        double slope_;
        double drift_;
        double t0_;
        adcontrols::chromatography::ePharmacopoeia pharmacopoeia_;
        adcontrols::chromatography::ePeakWidthMethod peakWidthMethod_;
        adcontrols::chromatography::ePeakWidthMethod theoreticalPlateMethod_;
        bool timeInMinutes_;
        std::vector< chromatography::TimedEvent > timedEvents_;
        chromatography::eNoiseFilterMethod noiseFilterMethod_;
        double cutoffFreqHz_; // Hz

        friend class PeakMethod_archive < PeakMethod > ;
        friend class PeakMethod_archive < const PeakMethod > ;
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version );
        friend ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const PeakMethod& );
        friend ADCONTROLSSHARED_EXPORT PeakMethod tag_invoke( const boost::json::value_to_tag< PeakMethod >&, const boost::json::value& jv );
	};

}

BOOST_CLASS_VERSION( adcontrols::chromatography::TimedEvent,  2 )
BOOST_CLASS_VERSION( adcontrols::PeakMethod,  4 )
