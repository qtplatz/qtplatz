/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include <vector>
#include <boost/smart_ptr.hpp>
#include <chromatogr/stack.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/baselines.hpp>

namespace adcontrols {
    class PeakMethod;
}

namespace chromatogr {

    //------- Intergrator ---------//

    class Integrator {
    public:
        Integrator(void);
        virtual ~Integrator(void);

        class chromatogram {
        public:
            double sampInterval_;
            double minTime_;
            std::vector<double> v_;
            inline const double * get() const { return &v_[0]; };
            inline size_t size() const { return v_.size(); };
            inline double getIntensity( long pos ) const { return v_[pos]; }
            inline double getTime( long pos ) const { return minTime_ + pos * sampInterval_; }
        };
        
    private:
        enum PEAKSTATE {
            PKUNDEF = (-1),
            PKTOP =  1,
            PKVAL =  2,
            PKSTA =  3,
            PKBAS =  4,
        };
        
        class PEAKSTACK {
            PEAKSTATE stat_;
            long pos_;
            double height_;
        public:
            PEAKSTACK(PEAKSTATE st, long tpos, double h) : stat_(st), pos_(tpos), height_(h) { /**/ };
        public:
            double height() const                      { return height_; };
            long pos() const                           { return pos_;    };
            PEAKSTATE stat() const                     { return stat_;   };
            bool operator == (PEAKSTATE st) const      { return stat_ == st; };
            bool operator != (PEAKSTATE st) const      { return stat_ != st; };
            void operator = (PEAKSTATE st)             { stat_ = st; };
        };

        chromatogram rdata_;
        stack< PEAKSTACK > stack_;
        adcontrols::Peaks peaks_;
        adcontrols::Baselines baselines_;

        std::vector<double> data0_;  // zero degree (noize reduced) data
        int posg_; // writing pointer + 1
        int posc_; // current pkfind pointer
        size_t ndata_;
        double data_[256];
        long dc_;  /* down count */
        long uc_;  /* up count */
        long zc_;  /* zero count */
        long lu_;  /* up start position */
        long ld_;  /* down start position */
        long lz_;  /* zero start position */
        double lud_;
        double ldd_;
        double lzd_;
        long stf_;
        long lockc_;
        long mw_;
        double ss_;
        unsigned long ndiff_;
        bool dirty_;
        long numAverage_;
        double minw_;
        double slope_;
        double drift_;
        bool detectSholder_;
        bool detectNegative_;
        bool offIntegration_;
        double timeOffset_;
        
    public:
        void samping_interval(double /* seconds */);
        void minimum_width(double /* seconds */);
        void slope_sensitivity(double /* uV / second */);
        void drift(double /* uV / second */);
        
        double currentTime() const;
        void timeOffset( double peaktime );
        
        void operator << ( double v );  // analogue input
        void close( const class adcontrols::PeakMethod&, adcontrols::Peaks& );
        void offIntegration( bool f ) { offIntegration_ = f; }
        bool offIntegration() const { return offIntegration_; }
        
    private:
        void update_params();
        
        void pkfind(long pos, double df1, double df2);
        void pktop(int f, int z);
        void pkbas(int t, double d);
        void pksta();
        void pkreduce();
        void pkcorrect(PEAKSTACK & sp, int f);
        bool intercept(const class adcontrols::Baseline& bs, long pos, double height);

        void assignBaseline();
        void reduceBaselines();
        void fixupPenetration( adcontrols::Baseline& );
        bool fixBaseline( adcontrols::Baseline&, adcontrols::Baselines& );
        void updatePeakAreaHeight( const adcontrols::PeakMethod& );
        void rejectPeaks( const adcontrols::PeakMethod& );
        void updatePeakParameters( const adcontrols::PeakMethod& );
        
        bool fixDrift( adcontrols::Peaks&, adcontrols::Baselines&, double drift );
        void remove( adcontrols::Baselines&, const adcontrols::Peaks& );
    };

}
