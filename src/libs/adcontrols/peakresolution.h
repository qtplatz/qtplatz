// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT PeakResolution {
    public:
        PeakResolution();
        PeakResolution( const PeakResolution& );
        double Rs() const;
        double RsBaselineStartTime() const;
        double RsBaselineStartHeight() const;
        double RsBaselineEndTime() const;
        double RsBaselineEndHeight() const;
        double RsPeakTopTime() const;
        double RsPeakTopHeight() const;

    private:
        double rs_;
        double rsBaselineStartTime_;
        double rsBaselineStartHeight_;
        double rsBaselineEndTime_;
        double rsBaselineEndHeight_;
        double rsPeakTopTime_;
        double rsPeakTopHeight_;

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP( rs_ );
                ar & BOOST_SERIALIZATION_NVP( rsBaselineStartTime_ );
                ar & BOOST_SERIALIZATION_NVP( rsBaselineStartHeight_ );
                ar & BOOST_SERIALIZATION_NVP( rsBaselineEndTime_ );
                ar & BOOST_SERIALIZATION_NVP( rsBaselineEndHeight_ );
                ar & BOOST_SERIALIZATION_NVP( rsPeakTopTime_ );
                ar & BOOST_SERIALIZATION_NVP( rsPeakTopHeight_ );
            }
        }

    };

}

