/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

//  Factor for N sigma value has been determined by following program.
//
//  #include <stdio.h>
//  #include <math.h>
//  #define GAUSE(u) ((1.0/(sqrt(2 * 3.141592))) * exp(-(u * u) / 2.0))
//  main()
//  {
//    printf("\n#define HEIGHT_2_SIGMA   %8.7lf", GAUSE(1.0) / GAUSE(0.0));
//    printf("\n#define HEIGHT_3_SIGMA   %8.7lf", GAUSE(1.5) / GAUSE(0.0));
//    printf("\n#define HEIGHT_4_SIGMA   %8.7lf", GAUSE(2.0) / GAUSE(0.0));
//    printf("\n#define HEIGHT_5_SIGMA   %8.7lf", GAUSE(2.5) / GAUSE(0.0));
//  }

#include <compiler/disable_unused_parameter.h>

#include "integrator.hpp"
#include "averager.hpp"
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/baseline.hpp>
#include <adportable/moment.hpp>
#include <adportable/polfit.hpp>
#include <adportable/profile.hpp>
#include <adportable/sgfilter.hpp>
#include <adportable/float.hpp>
#include <boost/bind.hpp>
#include <boost/numeric/interval.hpp>

#include <functional>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <deque>
#include <vector>
#include <set>
#include <cmath>
#include <cassert>
#include <memory>

#define HEIGHT_2_SIGMA   0.6065307
#define HEIGHT_3_SIGMA   0.3246525
#define HEIGHT_4_SIGMA   0.1353353
#define HEIGHT_5_SIGMA   0.0439369

namespace chromatogr {

    namespace integrator {

        class chromatogram {
        public:
            double sampInterval_;
            double minTime_;
            std::vector<double> v_;
            std::vector<double> t_;
            std::vector<double> d1_;
            
            inline const double * get() const {
                return v_.data();
            };

            inline size_t size() const {
                return v_.size();
            };

            inline double getIntensity( long pos ) const {
                if ( pos < 0 || pos >= size() )
                    return 0;
                return v_[pos];
            }

            inline double getTime( long pos ) const {
                if ( pos >= 0 && pos < t_.size() )
                    return t_.empty() ? minTime_ + pos * sampInterval_ : t_[ pos ];
                return 0;
            }
            chromatogram() : sampInterval_( 0 ), minTime_( 0 ) {}
        };
        
    }

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

    enum PEAK_START_STATE { PEAK_STATE_NEGATIVE = ( -1 ), PEAK_STATE_BASELINE = 0, PEAK_STATE_POSITIVE = 1 };
    
    class Integrator::impl {
    public:
        impl() : posg_( 0 )
               , posc_( 0 )
               , ndata_( 0 )
               , dc_(0) /* down count */
               , uc_(0) /* up count */
               , zc_(0) /* zero count */
               , lu_(0) /* up start position */
               , ld_(0) /* down start position */
               , lz_(0) /* zero start position */
               , lud_(0.0)
               , ldd_(0.0)
               , stf_( PEAK_STATE_BASELINE )
               , lockc_(0)
               , mw_(3)
               , ss_(0)
               , ndiff_(5)
               , dirty_(true)
               , numAverage_(3)
               , minw_( 0.5 /* second */)
               , slope_( 0.1 /* uV/s */ )
               , drift_( 0.0 /* uV/min */ )
               , detectSholder_(false)
               , detectNegative_(false)
               , offIntegration_(false)
               , timeOffset_(0) {
#if defined _DEBUG
            std::string file = adportable::profile::user_data_dir<char>() + "/data/integrator.txt";
            outf_.open( file );
#endif
        }
        ~impl() {
        }
        std::shared_ptr< adportable::SGFilter > sgd1_;
        
        integrator::chromatogram rdata_;
        
        stack< PEAKSTACK > stack_;

        adcontrols::Peaks peaks_;
        adcontrols::Baselines baselines_;

        std::vector<double> data0_;  // zero degree (noize reduced) data
        int posg_; // writing pointer + 1
        int posc_; // current pkfind pointer
        size_t ndata_;
        double data_[256];
        uint32_t dc_;  /* down count */
        uint32_t uc_;  /* up count */
        uint32_t zc_;  /* zero count */
        uint32_t lu_;  /* up start position */
        uint32_t ld_;  /* down start position */
        uint32_t lz_;  /* zero start position */
        double lud_;
        double ldd_;
        double lzd_;
        PEAK_START_STATE stf_;
        uint32_t lockc_;
        uint32_t mw_;
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
#if defined _DEBUG
        std::ofstream outf_;
#endif

        //
        double adddata( double time, double intensity );
        void update_params();
        void pkfind(long pos, double df1, double df2);
        void assign_vallay( size_t pos, size_t downpos );
        void assign_pktop( size_t pos, size_t uppos );
        // void pktop(int f, int z);
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

    class peakHelper {
    public:
        static void updateAreaHeight( const integrator::chromatogram& c, const adcontrols::Baseline& bs, adcontrols::Peak& pk );
        static bool tRetention_lsq( const integrator::chromatogram& c, adcontrols::Peak& pk );
        static bool tRetention_moment( const integrator::chromatogram& c, adcontrols::Peak& pk );
        static bool cleanup_baselines( const adcontrols::Peaks& pks, adcontrols::Baselines& bss );
        static adcontrols::Peak peak( const integrator::chromatogram& c, int spos, int tpos, int epos, unsigned long flags );
        static bool peak_width( const adcontrols::PeakMethod&, const integrator::chromatogram& c, adcontrols::Peak& pk );
        static bool asymmetry( const adcontrols::PeakMethod&, const integrator::chromatogram& c, adcontrols::Peak& pk );
        static bool theoreticalplate( const adcontrols::PeakMethod&, const integrator::chromatogram& c, adcontrols::Peak& pk );
    };

    class baselineHelper {
    public:
        static adcontrols::Baseline baseline( const integrator::chromatogram& c, int, int );
    };

}

using namespace chromatogr;

Integrator::~Integrator(void)
{
    delete impl_;
}

Integrator::Integrator(void) : impl_( new Integrator::impl() )
{
}

void
Integrator::samping_interval(double sampIntval /* seconds */)
{
    impl_->rdata_.sampInterval_ = sampIntval;
    impl_->update_params();
}

void
Integrator::minimum_width(double minw)
{
    impl_->minw_ = minw;
    impl_->update_params();
}

void
Integrator::slope_sensitivity(double slope)
{
    impl_->slope_ = slope;
    impl_->ss_ = slope;
}

void
Integrator::drift(double drift)
{
	impl_->drift_ = drift;
}

void
Integrator::timeOffset( double sec )
{
	impl_->rdata_.minTime_ = impl_->timeOffset_ = sec;
}

double
Integrator::currentTime() const
{
	return impl_->rdata_.minTime_ + impl_->posg_ * impl_->rdata_.sampInterval_;
}

void
Integrator::operator << ( const std::pair<double, double>& data )
{
    double d1 = impl_->adddata( data.first, data.second );
    impl_->pkfind( impl_->posc_, d1, 0 );
}

void
Integrator::operator << ( double adval )
{
    if ( impl_->posg_ == 0 ) {
        impl_->posc_ = impl_->numAverage_ / 2;
        impl_->sgd1_ = std::make_shared< adportable::SGFilter >( impl_->ndiff_, adportable::SGFilter::Derivative1, adportable::SGFilter::Cubic );
    }

    impl_->posg_++;

    double d0 = adval;
    double d1 = 0;

    Averager avgr(impl_->numAverage_);

    impl_->rdata_.v_.push_back(adval);  // raw data

    //  r[0][1][2][3][4]  := (size = 5)
    // d0[0][1][X]        := (create 3rd place.  First two data has to be estimated)
    // d1[0][X]

    if ( impl_->posg_ <= ( impl_->numAverage_ / 2 ) )
		impl_->data0_.push_back( d0 );

	if ( impl_->posg_ >= impl_->numAverage_ ) {
        d0 = avgr( &impl_->rdata_.v_[ impl_->posg_ - impl_->numAverage_ ] );
		impl_->data0_.push_back( d0 );
	}

    if ( impl_->posg_ >= long( impl_->ndiff_ + ( impl_->numAverage_ / 2 ) ) ) {
		int pos = impl_->posg_ - (impl_->ndiff_ + (impl_->numAverage_ / 2));
		d1 = (*impl_->sgd1_)( &impl_->data0_[ pos ] );
        //d2 = diff2( &impl_->data0_[ pos ] );
        impl_->pkfind( impl_->posc_++, d1, 0.0 );
	}
}

void
Integrator::close( const adcontrols::PeakMethod& mth, adcontrols::Peaks & peaks, adcontrols::Baselines& baselines )
{
	if ( !impl_->stack_.empty() ) {
		if ( (impl_->stack_[0] == PKTOP) || (impl_->stack_[0] == PKVAL) ) {
			impl_->stack_.push( PEAKSTACK(PKBAS, long(impl_->rdata_.size() - 1), 0) );
			impl_->pkreduce();
		}
	}

    impl_->assignBaseline();
    impl_->reduceBaselines();
    peakHelper::cleanup_baselines( impl_->peaks_, impl_->baselines_ );

    if ( impl_->fixDrift( impl_->peaks_, impl_->baselines_, impl_->drift_ ) ) {
        peakHelper::cleanup_baselines( impl_->peaks_, impl_->baselines_ );
        impl_->reduceBaselines();
	}

    impl_->updatePeakAreaHeight( mth );
    impl_->rejectPeaks( mth );
    peakHelper::cleanup_baselines( impl_->peaks_, impl_->baselines_ );

    impl_->updatePeakParameters( mth );

    peaks = impl_->peaks_;
	baselines = impl_->baselines_;
}

void
Integrator::offIntegration( bool flag )
{
    if ( ( impl_->offIntegration_ = flag ) ) {
        if ( !impl_->stack_.empty() ) {
            if ( (impl_->stack_[0] == PKTOP) || (impl_->stack_[0] == PKVAL) ) {
                impl_->stack_.push( PEAKSTACK(PKBAS, long(impl_->rdata_.size() - 1), 0) );
                impl_->pkreduce();
            }
        }
    }
}

void
Integrator::impl::updatePeakAreaHeight( const adcontrols::PeakMethod& )
{
    using adcontrols::Baselines;
    using adcontrols::Baseline;

    for ( adcontrols::Peaks::vector_type::iterator it = peaks_.begin(); it != peaks_.end(); ++it ) {
		long baseId = it->baseId();
		if ( baseId >= 0 ) { 
            Baselines::vector_type::iterator pbs = std::find_if(baselines_.begin(), baselines_.end(), boost::bind( &Baseline::baseId, _1 ) == baseId );
            if ( pbs != baselines_.end() )
                peakHelper::updateAreaHeight(rdata_, *pbs, *it);
		}
	}
}

void
Integrator::impl::rejectPeaks(const adcontrols::PeakMethod & mth)
{
    auto pos = std::remove_if( peaks_.begin(), peaks_.end(), [mth] ( const adcontrols::Peak& a ) {
            return a.peakArea() <= mth.minimumArea() || a.peakHeight() <= mth.minimumHeight();
        } );

    if ( pos != peaks_.end() )
        peaks_.erase( pos, peaks_.end() );
}


void
Integrator::impl::updatePeakParameters( const adcontrols::PeakMethod& method )
{
    using adcontrols::Baselines;
    using adcontrols::Peaks;
    using adcontrols::Peak;

    long id = 0;
    for ( Peaks::vector_type::iterator it = peaks_.begin(); it != peaks_.end(); ++it ) {

        Peak& pk = *it;
        pk.peakId( id++ );

        peakHelper::tRetention_lsq( rdata_, pk ) || peakHelper::tRetention_moment( rdata_, pk );

        // Peak width
        peakHelper::peak_width( method, rdata_, pk );

        // NTP
        peakHelper::theoreticalplate( method, rdata_, pk );        

		// TailingFactor
        peakHelper::asymmetry( method, rdata_, pk );

        // k'
		if ( method.t0() >= 0.001 )
			pk.capacityFactor( ( pk.peakTime() - method.t0() ) / method.t0() );
	}
}

void
Integrator::impl::pkfind( long pos, double df1, double )
{
	if ( offIntegration_ )
		return;

    if ( pos <= 2 )
        return;

    uint32_t mwup = int( ( minw_ / rdata_.sampInterval_ ) / 2 );
    if ( mwup < 2 )
        mwup = 2;

    uint32_t /*mwflat( mwup / 2 ),*/ mwdn( mwup );

    if ( stf_ > 0 ) {
        mwdn = 3;
    } else if ( stf_ < 0 ) {
        mwup = 3;
    }

    auto prev_stf = stf_;

    if ( df1 < -( ss_ ) ) {	/* Down slope */

        if ( dc_++ == 0 ) {
            ld_ = pos; // - mw_ / 2;
            ldd_ = rdata_.getIntensity( ld_ );
        }
        uc_ = 0;

    } else if ( df1 > ss_ ) {	/* UP slope */

        if ( uc_++ == 0 ) {
            lu_ = pos;
            lud_ = rdata_.getIntensity( ld_ );
        }
        dc_ = 0;

    }
    
    if ( mwup <= uc_ ) {	/* upslope found */
        stf_ = PEAK_STATE_POSITIVE;
        zc_ = 0;
    } else if ( mwdn <= dc_ ) {	/* downslope found */
        stf_ = PEAK_STATE_NEGATIVE;
        zc_ = 0;
    } else {
        if ( zc_++ == 0 )
            lz_ = pos;
        uint32_t count = std::max( uc_, dc_ ) + mw_ * 2;
        if ( zc_ > count && stf_ != PEAK_STATE_BASELINE )
            stf_ = PEAK_STATE_BASELINE;
    }
#if defined _DEBUG
    outf_ << pos << ",\t" << uc_ << ",\t" << dc_ << ",\t" << zc_ << ",\t" << rdata_.getIntensity( pos ) << ",\t" << df1 << std::endl;
#endif
    
    if ( stf_ != prev_stf ) {
        if ( prev_stf == PEAK_STATE_NEGATIVE && stf_ == PEAK_STATE_POSITIVE ) {
            assign_vallay( pos, ld_ );
        } else if ( prev_stf == PEAK_STATE_POSITIVE && stf_ == PEAK_STATE_NEGATIVE ) {
            assign_pktop( pos, lu_ );
        } else if ( prev_stf != PEAK_STATE_BASELINE && stf_ == PEAK_STATE_BASELINE ) {
            pkbas( lz_, lzd_ );		/* Peak end */
        } else {
            pksta();			/* Peak start */
        }
    }
}

void
Integrator::impl::assign_vallay( size_t pos, size_t ld )
{
	if ( ld > pos || rdata_.v_.size() <= pos || rdata_.v_.size() <= ld )
		return;
    auto it = std::min_element( rdata_.v_.begin() + ld, rdata_.v_.begin() + pos );
    auto vallay = std::distance( rdata_.v_.begin(), it );
    stack_.push( PEAKSTACK( PKVAL, long( vallay ), *it ) );
    pkreduce();
}

void
Integrator::impl::assign_pktop( size_t pos, size_t uppos )
{
	if ( uppos > pos || rdata_.v_.size() <= pos || rdata_.v_.size() <= uppos )
		return;
    auto it = std::max_element( rdata_.v_.begin() + uppos, rdata_.v_.begin() + pos );
    auto apex = std::distance( rdata_.v_.begin(), it );
    stack_.push( PEAKSTACK( PKTOP, long( apex ), *it ) );
}

void
Integrator::impl::pkbas(int t, double d)
{
    stack_.push( PEAKSTACK(PKBAS, t, d) );
    pkreduce();				/* Peak end */
}

void
Integrator::impl::pksta()
{
    uint32_t backtrack = 3;

    if ( stf_ == PEAK_STATE_NEGATIVE ) {
        while ( --backtrack && ld_ && rdata_.getIntensity( ld_ - 1 ) > rdata_.getIntensity( ld_ ) )
            --ld_;
        stack_.push( PEAKSTACK( PKSTA, ld_, rdata_.getIntensity( ld_ ) ) );
    }  else {
        while ( --backtrack && lu_ && rdata_.getIntensity( lu_ - 1 ) < rdata_.getIntensity( lu_ ) )
            --lu_;
        stack_.push( PEAKSTACK( PKSTA, lu_, rdata_.getIntensity( lu_ ) ) );
    }
}

void
Integrator::impl::pkreduce()
{
    PEAKSTACK sp0 = stack_.top();
 
#if defined _DEBUG 
    std::string s;
    for (int i = 0; i < int(stack_.size()); ++i) {
        if (stack_[i] == PKVAL)
            s += "V";
		if (stack_[i] == PKTOP)
			s += "T";
		if (stack_[i] == PKSTA)
			s += "S";
		if (stack_[i] == PKBAS)
			s += "B";
	}
    outf_ << "pkreduce: stack [" << s << "]" << std::endl;
#endif

	if (stack_.size() <= 1)	 {		/* stack empty */

		stack_.pop();
		return;

	} else if (stack_.size() == 2)	 {

		if ((sp0 == PKVAL) && (stack_[1] == PKSTA))	 { 	 /* SV */
			stack_.pop();
			stack_.pop();
			sp0 = PKSTA;
			stack_.push(sp0);
			return;
		} else if ((stack_[0] == PKBAS) && (stack_[1] == PKSTA)) { /* SB */
			stack_.pop();
			stack_.pop();
			return;
		}

	} else if (stack_.size() >= 3) {

		PEAKSTACK sp1 = stack_[1];
		PEAKSTACK sp2 = stack_[2];
		stack_.pop(); // remove sp0

		if (sp1 != PKTOP)	 {
			if (sp0 == PKBAS && sp2 == PKSTA)	 {  /* SVB */
				stack_.pop();  // remove sp1
				stack_.pop();  // remove sp2
                baselines_.add( baselineHelper::baseline( rdata_, sp2.pos(), sp1.pos() ) );
				return;
			} else	{
				return;
			}
		} else {
			unsigned long flags = sp2.stat() << 6 | sp1.stat() | sp0.stat();
            peaks_.add( peakHelper::peak( rdata_, sp2.pos(), sp1.pos(), sp0.pos(), flags ) );
		}

		stack_.pop();	// remove sp1, PKTOP

		if (sp2 == PKSTA && sp0 == PKBAS)	 { /* SxB */

            baselines_.add( baselineHelper::baseline( rdata_, sp2.pos(), sp0.pos() ) );
			stack_.pop();	// remove sp2, PKSTA

		} else if (sp2 == PKVAL && sp0 == PKBAS) { /* VxB */
			stack_.pop();	// remove sp2, PKVAL
			if (!stack_.empty()) {
				PEAKSTACK sp3 = stack_.top();
				if (sp3 == PKSTA)  { /* SVxB */
                    baselines_.add( baselineHelper::baseline( rdata_, sp3.pos(), sp0.pos() ) ); 
				}
				stack_.pop();  // remove sp3, PKSTA
			} else {
				// pkerror(sra, "REDUCE : Base did not correspond to Start"); 
			}
		} else if (sp2 == PKSTA && sp0 == PKVAL) { /* SxV */ 
			stack_.push(sp0);   // restore sp0
		} else if (sp2 == PKVAL && sp0 == PKVAL) { /* VxV */ 
			stack_.pop();		  //  remove sp2,  PKVAL
			stack_.push(sp0);  //  restore sp0, PKVAL
		}
	} 
}

bool
Integrator::impl::intercept(const adcontrols::Baseline & bs, long pos, double height)
{
	double h;
	long d = pos - bs.startPos();
	if (d) {
		h = (bs.stopHeight() - bs.startHeight()) * d / (bs.stopPos() - bs.startPos()) + bs.startHeight();
	} else {
		h = bs.startHeight();
	}
	return h > height;
}

bool
Integrator::impl::fixDrift( adcontrols::Peaks& pks, adcontrols::Baselines & bss, double drift )
{
    using adcontrols::Baselines;
    using adcontrols::Baseline;
    using adcontrols::Peaks;
    
    Baselines fixed( bss );

	for ( Baselines::vector_type::iterator it = bss.begin(); it != bss.end(); ++it ) {

		Baseline& bs = *it;
		double slope = ( bs.stopHeight() - bs.startHeight() ) / double( bs.stopPos() - bs.startPos() + 1 ) * rdata_.sampInterval_;

		if ( ( slope < 0 && drift < slope ) || ( slope > 0 && drift > slope ) ) { // negative || positive drift

			boost::numeric::interval<int> interval( bs.startPos(), bs.stopPos() );

			for ( Peaks::vector_type::iterator pkIt = pks.begin(); pkIt != pks.end(); ++pkIt ) {

				if ( boost::numeric::in<int>( pkIt->topPos(), interval ) ) {
					int id = fixed.add( baselineHelper::baseline( rdata_, pkIt->startPos(), pkIt->endPos() ) );
					pkIt->baseId( id );
				}

			}
		}

	}
	if ( bss.size() != fixed.size() ) {
		bss = fixed;
		return true;
	}
    return false;
}

bool
Integrator::impl::fixBaseline( adcontrols::Baseline& bs, adcontrols::Baselines& fixed )
{
	bool res = false;

    typedef adcontrols::Peaks::vector_type::iterator peak_iterator;
    std::vector< peak_iterator > peaks;

    for ( peak_iterator ipk = peaks_.begin(); ipk != peaks_.end(); ++ipk ) {
		if ( ipk->baseId() == bs.baseId() ) 
            peaks.push_back( ipk );
    }

    if ( ! peaks.empty() ) {   // if not emptry

		if (bs.startHeight() >= bs.stopHeight()) {  // negative slope : looking for backword direction

            std::vector<peak_iterator>::reverse_iterator fixup = peaks.rbegin();

            for ( std::vector<peak_iterator>::reverse_iterator ppk = peaks.rbegin(); ppk != peaks.rend() - 1; ++ppk) {

				if ( intercept(bs, (*ppk)->startPos(), (*ppk)->startHeight()) ) {

					long newId = fixed.add( baselineHelper::baseline(rdata_, (*ppk)->startPos(), bs.stopPos()) );

					// modify current baseline
					bs.stopPos((*ppk)->startPos());
					bs.stopTime((*ppk)->startTime());
					bs.stopHeight((*ppk)->startHeight());

                    while ( fixup != ppk )
                        (*fixup++)->baseId(newId);
					(*fixup++)->baseId( newId ); // := ppk
					res = true;
				}

			}

		} else { 
			// if slope is positive then check from front
            std::vector<peak_iterator>::iterator fixup = peaks.begin();

            for ( std::vector<peak_iterator>::iterator ppk = peaks.begin(); ppk != peaks.end() - 1; ++ppk) {

                if ( intercept(bs, (*ppk)->endPos(), (*ppk)->endHeight()) ) {

                    long newId = fixed.add( baselineHelper::baseline(rdata_, bs.startPos(), (*ppk)->endPos()) );

					// modify current baseline
                    while ( fixup != ppk )
                        (*fixup++)->baseId(newId);
					(*fixup++)->baseId( newId ); // := ppk

					bs.startPos( (*ppk)->endPos() );
					bs.startTime( (*ppk)->endTime() );
                    bs.startHeight( (*ppk)->endHeight() );

					res = true; //
				}
			}

        }
	}
	return res;
}

void
Integrator::impl::assignBaseline()
{
    using adcontrols::Baselines;
    using adcontrols::Baseline;
    using adcontrols::Peaks;
    using adcontrols::Peak;

    // Baselines & bss = basepeaks_.getBaselines();

	for ( Peaks::vector_type::iterator it = peaks_.begin(); it != peaks_.end(); ++it ) {
        Peak & pk = *it;
        // Baselines::vector_type::iterator pbs = Baseline::findPos( baselines_, pk.topPos() );
        Baselines::vector_type::iterator bsIt = std::find_if( baselines_.begin(), baselines_.end(), boost::bind( &Baseline::startPos, _1 ) >= pk.topPos() );
        if ( bsIt != baselines_.end() && bsIt->stopPos() < pk.topPos() ) {
			pk.baseId( bsIt->baseId() );
        } else {
            long id = baselines_.add( baselineHelper::baseline(rdata_, pk.startPos(), pk.endPos()) );
			pk.baseId(id);
		}
	}
}

void 
Integrator::impl::fixupPenetration( adcontrols::Baseline & bs)
{
    using adcontrols::Peaks;
    using adcontrols::Peak;

    if ( bs.startHeight() >= bs.stopHeight() ) {  // down slope, check back side

        Peaks::vector_type::reverse_iterator rpk = std::find_if( peaks_.rbegin(), peaks_.rend(), boost::bind( &Peak::baseId, _1 ) == bs.baseId() );
        long mpos = rpk->topPos() + ( rpk->endPos() - rpk->topPos() + 1 ) / 2;  // middle of right down slope
        long xpos = rpk->endPos();
        double dHmax = 0.0;
        
        for ( long n = rpk->endPos(); n >= mpos; --n ) {
            double dataH = rdata_.getIntensity( n );
            double baseH = bs.height( n );
            double deltaH  = dataH - baseH;
            if ( dHmax > deltaH ) {  // find where dHmax is minimum
                dHmax = deltaH;
                xpos = n;
            }
        }
        if ( xpos != rpk->endPos() ) {
            // move peak end point
            rpk->endPos( xpos, rdata_.getIntensity( xpos ) );
            rpk->endTime( rdata_.getTime( xpos ) );
            
            // move baseline stop point
            bs.stopPos( rpk->endPos() );
            bs.stopTime( rpk->endTime() );
            bs.stopHeight( rpk->endHeight() );
        }
        
    } else { // up slope, check front side
        adcontrols::Peaks::vector_type::iterator ipk = std::find_if( peaks_.begin(), peaks_.end(), boost::bind( &Peak::baseId, _1 ) == bs.baseId() );
        
        long mpos = ipk->startPos() + ( ipk->topPos() - ipk->startPos() + 1 ) / 2;  // middle of left up slope
        long xpos = ipk->startPos();
        double dHmax = 0;
        
        for (long n = ipk->startPos(); n <= mpos; ++n) {
            double dataH = rdata_.getIntensity(n);
            double baseH = bs.height(n);
            double deltaH = dataH - baseH;
            if ( dHmax < deltaH) {  // find where dHmax is minimum
                dHmax = deltaH;
                xpos = n;
            }
        }
        
        if ( xpos != ipk->startPos() ) {
            ipk->startPos( xpos, rdata_.getIntensity( xpos ) );
            ipk->startTime( rdata_.getTime( xpos ) );
            
            // move baseline stop point
            bs.startPos( ipk->endPos() );
            bs.startTime( ipk->endTime() );
            bs.startHeight( ipk->endHeight() );
        }
    }
}

void
Integrator::impl::reduceBaselines()
{
    using adcontrols::Baselines;

	Baselines fixed( baselines_ );

    bool bFixed = false;
    int idx = 0;
	for ( Baselines::vector_type::iterator ibs = baselines_.begin(); ibs != baselines_.end(); ++ibs, ++idx ) {
		if ( fixBaseline( *ibs, fixed ) ) {
            bFixed = true;
			Baselines::vector_type::iterator pos = fixed.begin() + idx;
			*pos = *ibs;
		}
	}
    if ( bFixed )
		baselines_ = fixed;
}

///////////////////////////////
// static
bool
peakHelper::tRetention_lsq(  const integrator::chromatogram& c, adcontrols::Peak& pk )
{
	double l_threshold = pk.topHeight() - ( (pk.topHeight() - pk.startHeight()) * 0.3 );
    double r_threshold = pk.topHeight() - ( (pk.topHeight() - pk.endHeight()) * 0.3 );

    // left boundary
    long left_bound = pk.topPos() - 1;
    while ( ( c.getIntensity( left_bound - 1 ) > l_threshold ) && ( ( left_bound - 1 ) > pk.startPos() ) )
        left_bound--;

    // right boundary
    long right_bound = pk.topPos() + 1;
    while ( ( c.getIntensity( right_bound + 1 ) > r_threshold ) && ( ( right_bound + 1 ) < pk.endPos() ) )
        right_bound++;

	if ( ( right_bound - left_bound + 1 ) < 5 ) {
		left_bound--;
		right_bound++;
	}

    std::vector<double> X, Y;
    for ( long i = left_bound; i <= right_bound; ++i ) {
        X.push_back( c.getTime( i ) );
        Y.push_back( c.getIntensity( i ) );
    }

    std::vector<double> r;
    if ( adportable::polfit::fit( &X[0], &Y[0], X.size(), 3, r ) ) {
        double a = r[0];
        double b = r[1];
        double c = r[2];
        double tR = (-b) / 2 / c;
		pk.peakTime( tR );
        (void)a;

        adcontrols::RetentionTime tr;
        tr.setAlgorithm( adcontrols::RetentionTime::ParaboraFitting );
        tr.setThreshold( l_threshold, r_threshold );
        tr.setBoundary( X[ 0 ], X[ X.size() - 1 ] );
        tr.setEq( a, b, c );
        pk.setRetentionTime( tr );
        
        return true;
    }
	return false;
}

namespace chromatogr { namespace internal {

        struct TimeFunctor {
            const integrator::chromatogram& c_;
            TimeFunctor( const integrator::chromatogram& c ) : c_( c ) { }
            double operator ()( int pos ) const { return c_.getTime( pos ); }
        };
    }
}

bool
peakHelper::tRetention_moment(  const integrator::chromatogram& c, adcontrols::Peak& pk )
{
    internal::TimeFunctor functor( c );
    adportable::Moment< internal::TimeFunctor > moment( functor );

    double h = pk.topHeight() - std::min( pk.startHeight(), pk.endHeight() );
    double threshold = pk.topHeight() - h * 0.5;

    double cx = moment.centreX( &c.v_[ 0 ], threshold, pk.startPos(), pk.topPos(), pk.endPos() );
    pk.peakTime( cx );

    adcontrols::RetentionTime tr;
    tr.setAlgorithm( adcontrols::RetentionTime::Moment );
    tr.setThreshold( threshold, threshold );
    tr.setBoundary( moment.xLeft(), moment.xRight() );

    pk.setRetentionTime( tr );
    return true;
}

namespace chromatogr {

    struct idNotFind {
        std::set<int>& idList;
        idNotFind( std::set<int>& t ) : idList(t) {}
        bool operator() ( const adcontrols::Baseline& bs ) {
            return idList.find ( bs.baseId() ) == idList.end();
        }
    };
}


bool
peakHelper::cleanup_baselines( const adcontrols::Peaks& pks, adcontrols::Baselines& bss )
{
	std::set<int> idList;

	for ( adcontrols::Peaks::vector_type::const_iterator it = pks.begin(); it != pks.end(); ++it )
		idList.insert( it->baseId() );

    adcontrols::Baselines::vector_type::iterator pos = std::remove_if( bss.begin(), bss.end(), idNotFind(idList) );
    if ( pos != bss.end() )
		bss.erase( pos, bss.end() );

	return true;
}

// static
void
peakHelper::updateAreaHeight( const integrator::chromatogram& c, const adcontrols::Baseline& bs, adcontrols::Peak& pk )
{
    double area = 0;
    double height = c.getIntensity( pk.topPos() ) - bs.height( pk.topPos() );
    for ( int pos = pk.startPos(); pos <= pk.endPos(); ++pos ) {
        double h = c.getIntensity(pos) - bs.height(pos);
        double w = c.getTime( pos + 1 ) - c.getTime( pos );
        if ( h >= 0.0 )
            area += h * w;
    }
    pk.peakArea( area );
    pk.peakHeight( height );
}

adcontrols::Peak
peakHelper::peak( const chromatogr::integrator::chromatogram& c, int spos, int tpos, int epos, unsigned long flags )
{
    adcontrols::Peak pk;

    pk.startPos( spos, c.getIntensity( spos ) );
    pk.topPos( tpos, c.getIntensity( tpos ) );
    pk.endPos( epos, c.getIntensity( epos ) );
    pk.peakFlags( flags );

    pk.startTime( c.getTime( spos ) );
    pk.peakTime( c.getTime( tpos ) );
    pk.endTime( c.getTime( epos ) );

    return pk;
}

bool
peakHelper::peak_width( const adcontrols::PeakMethod&, const integrator::chromatogram& c, adcontrols::Peak& pk )
{
    internal::TimeFunctor functor( c );
    adportable::Moment< internal::TimeFunctor > moment( functor );

    double threshold = pk.topHeight() - pk.peakHeight() * 0.5;
    double width = moment.width( &c.v_[0], threshold, pk.startPos(), pk.topPos(), pk.endPos() );
    
    pk.peakWidth( width );
    
    return true;
}

bool
peakHelper::asymmetry( const adcontrols::PeakMethod&, const integrator::chromatogram& c, adcontrols::Peak& pk )
{
    internal::TimeFunctor functor( c );
    adportable::Moment< internal::TimeFunctor > moment( functor );

    double threshold = pk.topHeight() - pk.peakHeight() * 0.95;
    double width = moment.width( &c.v_[0], threshold, pk.startPos(), pk.topPos(), pk.endPos() );
    double a = pk.peakTime() - moment.xLeft();

    adcontrols::PeakAsymmetry tf;

    tf.setAsymmetry( width / (2 * a) );
    tf.setBoundary( moment.xLeft(), moment.xRight() );

    pk.setAsymmetry( tf );

    return true;
}

bool
peakHelper::theoreticalplate( const adcontrols::PeakMethod&, const integrator::chromatogram& c, adcontrols::Peak& pk )
{
    internal::TimeFunctor functor( c );
    adportable::Moment< internal::TimeFunctor > moment( functor );

    double threshold = pk.topHeight() - pk.peakHeight() * 0.5;
    double width = moment.width( &c.v_[0], threshold, pk.startPos(), pk.topPos(), pk.endPos() );
    double N = 5.54 * ( ( pk.peakTime() / width ) * ( pk.peakTime() / width ) );

    adcontrols::TheoreticalPlate ntp;
    ntp.ntp( N );

    pk.setTheoreticalPlate( ntp );

    return true;
}

///////////////

adcontrols::Baseline
baselineHelper::baseline( const chromatogr::integrator::chromatogram& c, int spos, int epos )
{
    adcontrols::Baseline bs;

    bs.startPos( spos );
    bs.stopPos( epos );
    bs.startTime( c.getTime( spos ) );
    bs.stopTime( c.getTime( epos ) );
    bs.startHeight( c.getIntensity( spos ) );
    bs.stopHeight( c.getIntensity( epos ) ) ;

    return bs;
}

/////////////////////////////
void
Integrator::impl::update_params()
{
    mw_ = long ( minw_ / rdata_.sampInterval_ );
    if ( mw_ < 3 )
        mw_ = 3;
    mw_ |= 1;  // force 'odd'
    numAverage_ = mw_;
}

double
Integrator::impl::adddata( double time, double intensity )
{
    rdata_.t_.push_back( time );
    rdata_.v_.push_back( intensity );
    rdata_.d1_.push_back( 0 );
    
    if ( rdata_.t_.size() >= 2 ) {
        // estimate sampling interval (average)
        size_t advance = rdata_.t_.size() < 5 ? rdata_.t_.size() : 5;
        rdata_.sampInterval_ = ( rdata_.t_.back() - rdata_.t_[ rdata_.t_.size() - advance ] ) / advance; // last 'advance' points or all

        int mw = std::max( 5, int( minw_ / rdata_.sampInterval_ ) | 1 );
        if ( mw != mw_ ) {
            mw_ = mw;
            sgd1_.reset();
        }

    }

    if ( rdata_.size() >= mw_ ) {

        if ( ! sgd1_ )
            sgd1_ = std::make_shared< adportable::SGFilter >( mw_, adportable::SGFilter::Derivative1, adportable::SGFilter::Cubic );

        double d1 = ( *sgd1_ )( &rdata_.v_[ rdata_.size() - ( 1 + mw_ / 2 ) ] );

        posg_ = int( rdata_.size() - 1 );
        posc_ = int( rdata_.size() - ( mw_ / 2 ) - 1 );

        rdata_.d1_[ posc_ ] = d1;
        return d1;
    }
    return 0;
}
