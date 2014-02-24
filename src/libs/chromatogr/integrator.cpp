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
#include "differential.hpp"
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/baseline.hpp>
#include <adportable/moment.hpp>
#include <adportable/polfit.hpp>
#include <boost/bind.hpp>
#include <boost/numeric/interval.hpp>
#include <functional>
#include <algorithm>
#include <sstream>
#include <deque>
#include <vector>
#include <set>
#include <math.h>
#include <assert.h>

#define MAX_NDIM        7
#define MAXDIMENTION    (MAX_NDIM+1+2)  /* for 1-origin code + (mc = 2) */
// #define MAXDBUF 512  // should be power of 2

#define HEIGHT_2_SIGMA   0.6065307
#define HEIGHT_3_SIGMA   0.3246525
#define HEIGHT_4_SIGMA   0.1353353
#define HEIGHT_5_SIGMA   0.0439369

namespace chromatogr {

    class peakHelper {
    public:
        static void updateAreaHeight( const Integrator::chromatogram& c, const adcontrols::Baseline& bs, adcontrols::Peak& pk );
        static bool tRetention_lsq( const Integrator::chromatogram& c, adcontrols::Peak& pk );
        static bool tRetention_moment( const Integrator::chromatogram& c, adcontrols::Peak& pk );
        static bool cleanup_baselines( const adcontrols::Peaks& pks, adcontrols::Baselines& bss );
        static adcontrols::Peak peak( const Integrator::chromatogram& c, int spos, int tpos, int epos, unsigned long flags );
    };

    class baselineHelper {
    public:
        static adcontrols::Baseline baseline( const Integrator::chromatogram& c, int, int );
    };

}

using namespace chromatogr;

Integrator::~Integrator(void)
{
}

Integrator::Integrator(void) : posg_( 0 )
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
                             , lzd_(0.0)
                             , stf_(0)
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
                             , timeOffset_(0)
{
}

void
Integrator::update_params()
{
    mw_ = long ( minw_ / rdata_.sampInterval_ );
    if ( mw_ < 3 )
        mw_ = 3;
    mw_ |= 1;  // force 'odd'
    numAverage_ = mw_;
}

void
Integrator::samping_interval(double sampIntval /* seconds */)
{
    rdata_.sampInterval_ = sampIntval;
    update_params();
}

void
Integrator::minimum_width(double minw)
{
    minw_ = minw;
    update_params();
}

void
Integrator::slope_sensitivity(double slope)
{
    slope_ = slope;
    ss_ = slope_;
}

void
Integrator::drift(double drift)
{
	drift_ = drift;
}

void
Integrator::timeOffset( double sec )
{
	rdata_.minTime_ = timeOffset_ = sec;
}

double
Integrator::currentTime() const
{
	return rdata_.minTime_ + posg_ * rdata_.sampInterval_;
}

void
Integrator::operator << ( double adval )
{
	if ( posg_ == 0 )
		posc_ = numAverage_ / 2;

    posg_++;

    double d0 = adval;
    double d1 = 0;
    double d2 = 0;

    Averager avgr(numAverage_);
	differential<double> diff1(ndiff_);
	differential2<double> diff2(ndiff_);

    rdata_.v_.push_back(adval);  // raw data

    //  r[0][1][2][3][4]  := (size = 5)
    // d0[0][1][X]        := (create 3rd place.  First two data has to be estimated)
    // d1[0][X]

	if ( posg_ <= (numAverage_ / 2) )
		data0_.push_back( d0 );

	if ( posg_ >= numAverage_ ) {
		d0 = avgr( &rdata_.v_[ posg_ - numAverage_ ] );
		data0_.push_back( d0 );
	}

	if ( posg_ >= long (ndiff_ + (numAverage_ / 2)) ) {
		int pos = posg_ - (ndiff_ + (numAverage_ / 2));
		d1 = diff1( &data0_[ pos ] );
		d2 = diff2( &data0_[ pos ] );
		pkfind(posc_++, d1, d2);
	}
}

void
Integrator::close( const adcontrols::PeakMethod& mth, adcontrols::Peaks & peaks, adcontrols::Baselines& baselines )
{
	if ( !stack_.empty() ) {
		if ( (stack_[0] == PKTOP) || (stack_[0] == PKVAL) ) {
			stack_.push( PEAKSTACK(PKBAS, long(rdata_.size() - 1), 0) );
			pkreduce();
		}
	}

	assignBaseline();
	reduceBaselines();
    peakHelper::cleanup_baselines( peaks_, baselines_ );

	if ( fixDrift( peaks_, baselines_, drift_ ) ) {
        peakHelper::cleanup_baselines( peaks_, baselines_ );
		reduceBaselines();
	}

    updatePeakAreaHeight( mth );
	rejectPeaks( mth );
    peakHelper::cleanup_baselines( peaks_, baselines_ );

    updatePeakParameters( mth );

    peaks = peaks_;
	baselines = baselines_;
}

void
Integrator::updatePeakAreaHeight( const adcontrols::PeakMethod& )
{
    using adcontrols::Baselines;
    using adcontrols::Baseline;

    for ( adcontrols::Peaks::vector_type::iterator it = peaks_.begin(); it != peaks_.end(); ++it ) {
		long baseId = it->baseId();
		if ( baseId >= 0 ) { 
            Baselines::vector_type::iterator pbs = std::find_if( baselines_.begin(), baselines_.end(), boost::bind( &Baseline::baseId, _1 ) == baseId );
			if ( pbs != baselines_.end() )
                peakHelper::updateAreaHeight(rdata_, *pbs, *it);
		}
	}
}

void
Integrator::rejectPeaks(const adcontrols::PeakMethod & mth)
{
	double minArea = mth.minimumArea();
    using adcontrols::Peaks;
    using adcontrols::Peak;

	if (minArea > 0.0) {
		Peaks::vector_type::iterator pos = std::remove_if( peaks_.begin(), peaks_.end(), boost::bind( &Peak::peakArea, _1 ) < minArea );
        peaks_.erase( pos, peaks_.end() );
	}

	double minHeight = mth.minimumHeight();
	if (minHeight > 0.0) {
		Peaks::vector_type::iterator pos = std::remove_if( peaks_.begin(), peaks_.end(), boost::bind( &Peak::peakHeight, _1 ) < minHeight );
		peaks_.erase( pos, peaks_.end() );
	}
}


void
Integrator::updatePeakParameters( const adcontrols::PeakMethod& method )
{
    using adcontrols::Baselines;
    using adcontrols::Peaks;
    using adcontrols::Peak;

    long id = 0;
    for ( Peaks::vector_type::iterator it = peaks_.begin(); it != peaks_.end(); ++it ) {

        Peak& pk = *it;
        pk.peakId( id++ );

        peakHelper::tRetention_lsq( rdata_, pk ) || peakHelper::tRetention_moment( rdata_, pk );

        // TODO: NTP
        // Peak width
		// TailingFactor

        // k'
		if ( method.t0() >= 0.001 )
			pk.capacityFactor( ( pk.peakTime() - method.t0() ) / method.t0() );
	}
}

void
Integrator::pkfind(long pos, double df1, double df2)
{
    (void)df2;
	int	sstf;
	int	mwup, mwflat, mwdn;

	if ( offIntegration_ )
		return;
  
	mwup = mw_;
	if (mwup < 3)
		mwup = 3;
	else if (mwup > 32)
		mwup = 32;
	mwflat = mwdn = mwup;
	if (stf_ > 0) {
		mwflat = mw_ / 2;
		mwdn = 1;
	} else if (stf_ < 0) {
		mwflat = mw_ / 2;
		mwup = 1;
	}
	sstf = stf_;
	if (df1 < -(ss_))	 {	/* Down slope */
		if (dc_ == 0) {
			ld_ = pos;
			ldd_ = rdata_.getIntensity( ld_ );
		}
		++dc_;
		uc_ = zc_ = 0;
	} else if (df1 > ss_)	 {	/* UP slope */
		if (uc_ == 0) {
			lu_ = pos;
			lud_ = rdata_.getIntensity( ld_ );
		}
		++uc_;
		dc_ = zc_ = 0;
	} else {
		if (zc_ == 0) { 
			lz_ = pos;
			lzd_ = rdata_.getIntensity( ld_ );
		}
		++zc_;
	}
	if (mwup <= uc_)	 	{	/* upslope found */
		stf_ = 1;
	} else if (mwdn <= dc_)	 {	/* downslope found */
		stf_ = -1;
	}
	if (mwflat <= zc_)	 {	/* baslien founded */
		stf_ = 0;
		uc_ = dc_ = 0;
	}

	if (stf_ != sstf)	 {
		if (sstf < 0 && stf_ > 0)	 {
			pktop(-1, pos - ld_);		/* Valley detect */
		} else if (sstf > 0 && stf_ < 0)	 {
			pktop(1,  pos - lu_);		/* Top detect */
		} else if (sstf && !stf_)	 {
			pkbas(lz_, lzd_);		/* Peak end */
		} else {
			pksta();			/* Peak start */
		}
	}
}

void
Integrator::pktop(int f, int z)
{
    if (f < 0) { // VALLAY
        long pos = lu_ + 1;
        while (pos && --z >= 0) {
            double d = rdata_.getIntensity( pos );
            if (d < lud_) {
                lud_ = d;
                lu_ = pos;
            }
            --pos;
        }
        stack_.push( PEAKSTACK(PKVAL, lu_, lud_) );
        pkreduce();
    } else {  // TOP
        long pos = ld_ + 1;
        while (pos && --z >= 0) {
            double d = rdata_.getIntensity( pos );
            if (d >= ldd_) {
                ldd_ = d;
                ld_ = pos;
            }
            --pos;
        }
        stack_.push( PEAKSTACK(PKTOP, ld_, ldd_) );
    }
}

void
Integrator::pkbas(int t, double d)
{
    stack_.push( PEAKSTACK(PKBAS, t, d) );
    pkreduce();				/* Peak end */
}

void
Integrator::pksta()
{
    if (stf_ < 0)
        stack_.push( PEAKSTACK(PKSTA, ld_, ldd_) );
    else
        stack_.push( PEAKSTACK(PKSTA, lu_, lud_) );
}

void
Integrator::pkreduce()
{
    PEAKSTACK sp0 = stack_.top();
 
#if defined _DEBUG && 0
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
	debug_trace(LOG_DEBUG, "pkreduce: stack [%s]", s.c_str(), 0);
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
			peaks_.add( peakHelper::peak(rdata_, sp2.pos(), sp1.pos(), sp0.pos(), flags) ); 
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
Integrator::intercept(const adcontrols::Baseline & bs, long pos, double height)
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
Integrator::fixDrift( adcontrols::Peaks& pks, adcontrols::Baselines & bss, double drift )
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
Integrator::fixBaseline( adcontrols::Baseline& bs, adcontrols::Baselines& fixed )
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
Integrator::assignBaseline()
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
Integrator::fixupPenetration( adcontrols::Baseline & bs)
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
Integrator::reduceBaselines()
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
peakHelper::tRetention_lsq(  const Integrator::chromatogram& c, adcontrols::Peak& pk )
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
        return true;
    }
	return false;
}

namespace chromatogr { namespace internal {

        struct TimeFunctor {
            const Integrator::chromatogram& c_;
            TimeFunctor( const Integrator::chromatogram& c ) : c_( c ) { }
            double operator ()( int pos ) const { return c_.getTime( pos ); }
        };
    }
}

bool
peakHelper::tRetention_moment(  const Integrator::chromatogram& c, adcontrols::Peak& pk )
{
    internal::TimeFunctor functor( c );
    adportable::Moment< internal::TimeFunctor > moment( functor );

    double cx = moment.centerX( &c.v_[0], 0.5, pk.startPos(), pk.topPos(), pk.endPos() );
    pk.peakTime( cx );
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
peakHelper::updateAreaHeight( const Integrator::chromatogram& c, const adcontrols::Baseline& bs, adcontrols::Peak& pk )
{
    double area = 0;
    double height = c.getIntensity( pk.topPos() ) - bs.height( pk.topPos() );
    for ( int pos = pk.startPos(); pos <= pk.endPos(); ++pos ) {
        double h = c.getIntensity(pos) - bs.height(pos);
        double w = c.getTime( pos + 1 );
        area += h * w;
    }
    pk.peakArea( area );
    pk.peakHeight( height );
}

adcontrols::Peak
peakHelper::peak( const chromatogr::Integrator::chromatogram& c, int spos, int tpos, int epos, unsigned long flags )
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

///////////////

adcontrols::Baseline
baselineHelper::baseline( const chromatogr::Integrator::chromatogram& c, int spos, int epos )
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
