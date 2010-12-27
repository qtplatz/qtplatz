// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <string>
#include "timeutil.h"
#include "peakasymmetry.h"
#include "peakresolution.h"
#include "theoreticalplate.h"

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
        // long UserData() const;
        // void UserData(long);

        void peakFlags(unsigned long);
        unsigned long peakFlags() const;

        const std::wstring& name() const;
        void  name(const std::wstring& );
        long  appliedFunctions() const;
        long  startPos() const;
        long  topPos() const;
        long  endPos() const;

        void  startPos(long, const class CMCChromatogram &);
        void  topPos(long, const class CMCChromatogram &);
        void  endPos(long, const class CMCChromatogram &);

        void  startPos(long pos, peakheight_t h);
        void  topPos(long pos,   peakheight_t h);
        void  endPos(long pos,   peakheight_t h);

        seconds_t startTime() const;
        void   startTime( seconds_t newTime);
        seconds_t topTime() const;
        void   topTime( seconds_t newTime);
        seconds_t endTime() const;
        void   endTime( seconds_t newTime);

        double startHeight() const;
        double topHeight() const;
        double endHeight() const;

        double peakArea() const;
        double peakHeight() const;

        double CapacityFactor() const;
        double PeakWidth() const;
        double PeakAmount() const;
        double MigrationTime() const;
        double PeakEfficiency() const;
        double MassOnColumn() const;

        double PercentArea() const;
        double PercentHeight() const;
        bool IsManuallyModified() const;
        void SetManuallyModified();


    private:
        std::wstring name_;
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
        seconds_t topTime_;
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

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if ( version >= 0 ) {
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
                ar & BOOST_SERIALIZATION_NVP( topTime_ );
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
            }
        }

    };

}

