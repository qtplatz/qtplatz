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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT PeakAsymmetry {
    public:
        PeakAsymmetry();
        PeakAsymmetry( const PeakAsymmetry& );
        double asymmetry() const;
        double startTime() const;
        double endTime() const;

        void setAsymmetry( double );
        void setBoundary( double, double );

    private:
        double peakAsymmetry_;
        double peakAsymmetryStartTime_;
        double peakAsymmetryEndTime_;

    private:
        friend ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const PeakAsymmetry& );
        friend ADCONTROLSSHARED_EXPORT PeakAsymmetry tag_invoke( const boost::json::value_to_tag< PeakAsymmetry >&, const boost::json::value& jv );

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
	    (void)version;
	    ar & BOOST_SERIALIZATION_NVP( peakAsymmetry_ );
	    ar & BOOST_SERIALIZATION_NVP( peakAsymmetryStartTime_ );
	    ar & BOOST_SERIALIZATION_NVP( peakAsymmetryEndTime_ );
        }

    };

}
