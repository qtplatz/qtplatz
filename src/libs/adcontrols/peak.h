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
/*
        double NTP() const;
        double NTPBaselineStartTime() const;
        double NTPBaselineStartHeight() const;
        double NTPBaselineEndTime() const;
        double NTPBaselineEndHeight() const;
        double NTPPeakTopTime() const;
        double NTPPeakTopHeight() const;
*/
/*
        double Resolution() const;
        double RsBaselineStartTime() const;
        double RsBaselineStartHeight() const;
        double RsBaselineEndTime() const;
        double RsBaselineEndHeight() const;
        double RsPeakTopTime() const;
        double RsPeakTopHeight() const;
*/
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

    protected:
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

    };

}

