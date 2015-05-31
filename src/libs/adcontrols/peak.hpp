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
#include <string>
#include "timeutil.hpp"
#include "peakasymmetry.hpp"
#include "peakresolution.hpp"
#include "theoreticalplate.hpp"
#include "retentiontime.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT Peak {
    public:
        ~Peak();
        Peak();
        Peak( const Peak& );
        typedef double peakheight_t;
    public:
        long parentId() const;
        void parentId(long id);
        long baseId() const;
        void baseId(long id);
        long peakId() const;
        void peakId(long id);

        void peakFlags(unsigned long);
        unsigned long peakFlags() const;

        const std::wstring& name() const;
        void  name(const std::wstring& );

        const char * formula() const;
        void  formula(const char * );
           
        long  appliedFunctions() const;
        void  appliedFunctions( long );
        long  startPos() const;
        long  topPos() const;
        long  endPos() const;

        void  startPos(long pos, peakheight_t h);
        void  topPos(long pos,   peakheight_t h);
        void  endPos(long pos,   peakheight_t h);

        seconds_t startTime() const;
        void   startTime( seconds_t newTime);
        seconds_t peakTime() const;
        void   peakTime( seconds_t newTime);
        seconds_t endTime() const;
        void   endTime( seconds_t newTime);

        double startHeight() const;
        double topHeight() const;
        double endHeight() const;

        double peakArea() const;
        void peakArea( double );

        double peakHeight() const;
        void peakHeight( double );

        double capacityFactor() const;
        void capacityFactor( double );

        double peakWidth() const;
        void peakWidth( double );

        double peakAmount() const;
        void peakAmount( double );

        double peakEfficiency() const;
        void peakEfficiency( double );

        double percentArea() const;
        void percentArea( double );

        double percentHeight() const;
        void percentHeight( double );

        bool isManuallyModified() const;
        void manuallyModified( bool );

        const PeakAsymmetry& asymmetry() const;
        const PeakResolution& resolution() const;
        const TheoreticalPlate& theoreticalPlate() const;
        const RetentionTime& retentionTime() const;

        void setAsymmetry( const adcontrols::PeakAsymmetry& );
        void setResolution( const PeakResolution& );
        void setTheoreticalPlate( const TheoreticalPlate& );
        void setRetentionTime( const RetentionTime& );

        void userData( uint64_t );
        uint64_t userData() const;

    private:
#if defined _MSC_VER
# pragma warning( disable: 4251 )
#endif
        std::wstring name_;
        std::string formula_;
        long parentId_;
        long peakid_;
        long baseid_;
        long appliedFunctions_;
        unsigned long peak_flags_;  // pair<front:3, rear:3>
        long startPos_;
        long topPos_;
        long endPos_;
    protected:
        seconds_t startTime_;
        seconds_t peakTime_;
        seconds_t endTime_;
        seconds_t startHeight_;
        seconds_t topHeight_;
        seconds_t endHeight_;
        double peakArea_;
        double peakHeight_;
        adcontrols::PeakAsymmetry asymmetry_;
        PeakResolution rs_;
        TheoreticalPlate ntp_;
        double capacityFactor_;
        double peakWidth_;
        double peakAmount_;
        double migrationTime_;
        double peakEfficiency_;
        double massOnColumn_;
        double percentArea_;
        double percentHeight_;
        bool manuallyModified_;
        RetentionTime tr_;
        uint64_t userData_;

    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( name_ );
            ar & BOOST_SERIALIZATION_NVP( parentId_ );
            ar & BOOST_SERIALIZATION_NVP( peakid_ );
            ar & BOOST_SERIALIZATION_NVP( baseid_ );
            ar & BOOST_SERIALIZATION_NVP( appliedFunctions_ );
            ar & BOOST_SERIALIZATION_NVP( peak_flags_ );  // pair<front:3, rear:3>
            ar & BOOST_SERIALIZATION_NVP( startPos_ );
            ar & BOOST_SERIALIZATION_NVP( topPos_ );
            ar & BOOST_SERIALIZATION_NVP( endPos_ );
            ar & BOOST_SERIALIZATION_NVP( startTime_ );
            ar & BOOST_SERIALIZATION_NVP( peakTime_ );
            ar & BOOST_SERIALIZATION_NVP( endTime_ );
            ar & BOOST_SERIALIZATION_NVP( startHeight_ );
            ar & BOOST_SERIALIZATION_NVP( topHeight_ );
            ar & BOOST_SERIALIZATION_NVP( endHeight_ );
            ar & BOOST_SERIALIZATION_NVP( peakArea_ );
            ar & BOOST_SERIALIZATION_NVP( peakHeight_ );
            ar & BOOST_SERIALIZATION_NVP( asymmetry_ );
            ar & BOOST_SERIALIZATION_NVP( rs_ );
            ar & BOOST_SERIALIZATION_NVP( ntp_ );
            ar & BOOST_SERIALIZATION_NVP( capacityFactor_ );
            ar & BOOST_SERIALIZATION_NVP( peakWidth_ );
            ar & BOOST_SERIALIZATION_NVP( peakAmount_ );
            ar & BOOST_SERIALIZATION_NVP( migrationTime_ );
            ar & BOOST_SERIALIZATION_NVP( peakEfficiency_ );
            ar & BOOST_SERIALIZATION_NVP( massOnColumn_ );
            ar & BOOST_SERIALIZATION_NVP( percentArea_ );
            ar & BOOST_SERIALIZATION_NVP( percentHeight_ );
            ar & BOOST_SERIALIZATION_NVP( manuallyModified_ );
            if ( version >= 1 )
                ar & BOOST_SERIALIZATION_NVP( tr_ );
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP( formula_ );
                ar & BOOST_SERIALIZATION_NVP( userData_ );
            }
        }

    };

}

BOOST_CLASS_VERSION( adcontrols::Peak, 2 )
