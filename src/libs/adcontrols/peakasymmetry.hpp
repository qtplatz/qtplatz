// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
//#include <boost/serialization/string.hpp>
//#include <boost/serialization/vector.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT PeakAsymmetry {
    public:
        PeakAsymmetry();
        PeakAsymmetry( const PeakAsymmetry& );
        double asymmetry() const;
        double startTime() const;
        double endTime() const;

        void asymmetry( double );
        void startTime( double value );
        void endTime( double value );

    private:
        double peakAsymmetry_;
        double peakAsymmetryStartTime_;
        double peakAsymmetryEndTime_;

    private:
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

