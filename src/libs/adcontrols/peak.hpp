// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
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
#include <cstdint>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT Peak {
    public:
        ~Peak();
        Peak();
        Peak( const Peak& );
        typedef double peakheight_t;
    public:
        std::int32_t parentId() const;
        void setParentId(std::int32_t id);
        
        std::int32_t baseId() const;
        void setBaseId(std::int32_t id);
        
        std::int32_t peakId() const;
        void setPeakId(std::int32_t id);

        std::uint32_t peakFlags() const;
        void setPeakFlags(std::uint32_t);

        const std::string& name() const;
        void  setName(const std::string& );

        const char * formula() const;
        void  setFormula(const char * );
           
        std::int32_t  appliedFunctions() const;
        void  setAppliedFunctions( std::int32_t );
        
        std::int32_t  startPos() const;
        std::int32_t  topPos() const;
        std::int32_t  endPos() const;

        void  setStartPos(std::int32_t pos, peakheight_t h);
        void  setTopPos(std::int32_t pos,   peakheight_t h);
        void  setEndPos(std::int32_t pos,   peakheight_t h);

        seconds_t startTime() const;
        void   setStartTime( seconds_t newTime);

        seconds_t peakTime() const;
        void   setPeakTime( seconds_t newTime );
        
        seconds_t endTime() const;
        void   setEndTime( seconds_t newTime);

        double startHeight() const;
        double topHeight() const;
        double endHeight() const;

        double peakArea() const;
        void setPeakArea( double );

        double peakHeight() const;
        void setPeakHeight( double );

        double capacityFactor() const;
        void setCapacityFactor( double );

        double peakWidth() const;
        void setPeakWidth( double );

        double peakAmount() const;
        void setPeakAmount( double );

        double peakEfficiency() const;
        void setPeakEfficiency( double );

        double percentArea() const;
        void setPercentArea( double );

        double percentHeight() const;
        void setPercentHeight( double );

        bool isManuallyModified() const;
        void setManuallyModified( bool );

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
        std::string name_;          // UTF-8
        std::string formula_;
        std::int32_t parentId_;
        std::int32_t peakid_;
        std::int32_t baseid_;
        std::int32_t appliedFunctions_;
        std::uint32_t peak_flags_;  // pair<front:3, rear:3>
        std::int32_t startPos_;
        std::int32_t topPos_;
        std::int32_t endPos_;
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
            if ( version < 3 ) {
                std::wstring name;
                ar & BOOST_SERIALIZATION_NVP( name ); // ignore it
            } else {
                ar & BOOST_SERIALIZATION_NVP( name_ );
            }
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

BOOST_CLASS_VERSION( adcontrols::Peak, 3 )
