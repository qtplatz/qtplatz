/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include "integrator.hpp"
#include "averager.hpp"
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/baseline.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/moment.hpp>
#include <adportable/polfit.hpp>
#include <adportable/profile.hpp>
#include <adportable/sgfilter.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/numeric/interval.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <fstream>
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <set>
#include <sstream>
#include <vector>

#define HEIGHT_2_SIGMA   0.6065307
#define HEIGHT_3_SIGMA   0.3246525
#define HEIGHT_4_SIGMA   0.1353353
#define HEIGHT_5_SIGMA   0.0439369

namespace {

    using adportable::SGFilter;

    class signal_processor {
        bool isCounting_;
        size_t navg_;
        size_t ndiff_;
        size_t posc_;
        SGFilter sgfilter_;
        enum { TIME, RAW_INTENSITY, AVERAGED_INTENSITY, DERIVERTIVE1 };
        std::vector< std::tuple< double, double, double, double > > d_;
    public:
        signal_processor( bool isCounting
                          , size_t navg
                          , size_t ndiff ) : isCounting_( isCounting )
                                           , navg_( navg )
                                           , ndiff_( ndiff )
                                           , posc_( 0 )
                                           , sgfilter_( ndiff, SGFilter::Derivative1, SGFilter::Cubic ) {
        }

        signal_processor& operator << ( std::pair<double, double >&& v ) {
            d_.emplace_back( std::get< 0 >( v ), std::get< 1 >( v ), 0.0, 0.0 );
            std::get< AVERAGED_INTENSITY >( d_.back() )
                = std::accumulate( d_.end() - std::min(navg_, d_.size()), d_.end(), 0.0
                                   , []( const auto& a, const auto& b ){ return a + std::get< RAW_INTENSITY >( b ); } )
                / std::min(navg_, d_.size());

            if ( d_.size() >= ndiff_ ) {
                posc_ = d_.size() - ( 1 + ndiff_ / 2 );
#if 0
                std::get< DERIVERTIVE1 >( d_[ posc_ ] ) =
                    sgfilter_( [&]( size_t i ){ return std::get< RAW_INTENSITY >( d_[ i ] );}, posc_ );
#else
                std::get< DERIVERTIVE1 >( d_[ posc_ ] ) =
                    sgfilter_( [&]( size_t i ){ return std::get< AVERAGED_INTENSITY >( d_[ i ] );}, posc_ );
#endif
            }
            return *this;
        }
        bool isCounting() const { return isCounting_; }
        size_t pos_g() const {  return d_.empty() ? 0 : d_.size() - 1; }
        size_t pos_c() const {  return posc_;   }

        void set_ndiff( size_t ndiff ) {
            assert( ndiff < 2048 );
            if ( ndiff != ndiff_ ) {
                ndiff_ = ndiff;
                sgfilter_ = SGFilter( ndiff, SGFilter::Derivative1, SGFilter::Cubic );
            }
        }
        void set_number_of_average( size_t n ) { navg_ = n; }
        size_t ndiff() const { return ndiff_; }

        const std::tuple< double, double, double, double >& operator [] ( size_t i ) const { return d_.at( i ); }
        const std::tuple< double, double, double, double >& at( size_t i ) const { return d_.at( i ); }
        const std::vector< std::tuple< double, double, double, double > >& d() const { return d_; }

        double getIntensity( size_t pos ) const { return pos < d_.size() ? std::get< RAW_INTENSITY >( d_[ pos ] ) : 0; }
        double getTime( size_t pos ) const { return pos < d_.size() ? std::get< TIME >( d_[ pos ] ) : 0; }
        double sampInterval() const {
            return d_.size() >= 2 ? ( std::get< TIME >( d_.back() ) - std::get< TIME >( d_.front() ) ) / (d_.size() - 1) : 0;
        }

        static inline double d1( const std::tuple< double, double, double, double > & t ) {
            return std::get< DERIVERTIVE1 >( t );
        }
        static inline double time( const std::tuple< double, double, double, double > & t ) {
            return std::get< TIME >( t );
        }
        static inline double value( const std::tuple< double, double, double, double >& t ) {
            return std::get< RAW_INTENSITY >( t );
        }
    };
}

namespace chromatogr {

    enum PEAKSTATE {
        PKUNDEF = 0,
        PKTOP =  1,
        PKVAL =  2,
        PKSTA =  3,
        PKBAS =  4,
    };

    char toChar( PEAKSTATE s ) {
        static const char __state_names[] = { 'x', 'T', 'V', 'S', 'B' };
        return s < (sizeof(__state_names)/sizeof(__state_names[0])) ? __state_names[ s ] : 'X';
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
        impl( bool isCounting ) : signal_processor_( std::make_unique< signal_processor >( isCounting, 3, 5 ) )
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
                                , timeOffset_(0)
                                , isCounting_( isCounting )  {
#if defined _DEBUG
            std::string file = adportable::profile::user_data_dir<char>() + "/data/integrator.txt";
            outf_.open( file );
#endif
        }
        ~impl() {
        }

        std::unique_ptr< signal_processor > signal_processor_;
        // chromatogram rdata_;
        std::optional< double > sampInterval_; // seconds

        stack< PEAKSTACK > stack_;

        adcontrols::Peaks peaks_;
        adcontrols::Baselines baselines_;

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
        int32_t mw_;
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
        bool isCounting_;
#if defined _DEBUG
        std::ofstream outf_;
#endif
        int pos_g() { return signal_processor_->pos_g(); }
        int pos_c() { return signal_processor_->pos_c(); }
        //
        void update_mw();
        void update_params();
        void pkfind(long pos, double df1, double df2);
        void assign_valley( size_t pos, size_t downpos );
        void assign_pktop( size_t pos, size_t uppos );
        // void pktop(int f, int z);
        void pkbas(int t, double d);
        void pksta();
        void pkreduce();
        void pkcorrect(PEAKSTACK & sp, int f);
        bool intercept(const class adcontrols::Baseline& bs, long pos, double height);
        void assignBaseline();
        void reduceBaselines();
        void fixPenetration( adcontrols::Baseline&, const adcontrols::Peak& );
        // void fixupPenetration( adcontrols::Baseline& );
        bool fixBaseline( adcontrols::Baseline&, adcontrols::Baselines& );
        void updatePeakAreaHeight( const adcontrols::PeakMethod& );
        void rejectPeaks( const adcontrols::PeakMethod& );
        void updatePeakParameters( const adcontrols::PeakMethod& );
        bool fixDrift( adcontrols::Peaks&, adcontrols::Baselines&, double drift );
        void remove( adcontrols::Baselines&, const adcontrols::Peaks& );
    };
}

namespace {

    using chromatogr::PEAKSTACK;

    class helper {
    public:
        static adcontrols::Peak peak( const signal_processor& c, int spos, int tpos, int epos, unsigned long flags );
        static adcontrols::Peak peak( const signal_processor& c, const PEAKSTACK& s, const PEAKSTACK& t, const PEAKSTACK& e );
        static adcontrols::Baseline baseline( const signal_processor& c, int, int );

        static void updateAreaHeight( const signal_processor& c, const adcontrols::Baseline& bs, adcontrols::Peak& pk );
        static bool tRetention_lsq( const signal_processor& c, adcontrols::Peak& pk );
        static bool tRetention_moment( const signal_processor& c, adcontrols::Peak& pk );
        static bool cleanup_baselines( const adcontrols::Peaks& pks, adcontrols::Baselines& bss );
        static bool peak_width( const adcontrols::PeakMethod&, const signal_processor& c, adcontrols::Peak& pk );
        static bool asymmetry( const adcontrols::PeakMethod&, const signal_processor& c, adcontrols::Peak& pk );
        static bool theoreticalplate( const adcontrols::PeakMethod&, const signal_processor& c, adcontrols::Peak& pk );
    };
}

using namespace chromatogr;

Integrator::~Integrator(void)
{
    delete impl_;
}

Integrator::Integrator( bool isCounting ) : impl_( new Integrator::impl( isCounting ) )
{
}

void
Integrator::sampling_interval(double sampIntval /* seconds */)
{
    impl_->sampInterval_ = sampIntval;
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

std::pair< double, double >
Integrator::currentTime() const
{
    return { impl_->signal_processor_->getTime( impl_->signal_processor_->pos_g() )
             , ( impl_->signal_processor_->pos_g() + 1 ) * impl_->signal_processor_->sampInterval() };
	// return impl_->rdata_.minTime_ + impl_->posg_ * impl_->rdata_.sampInterval_;
}

void
Integrator::operator << ( std::pair<double, double>&& data )
{
    (*impl_->signal_processor_) << std::move( data );
    double df1 = impl_->signal_processor_->pos_c() ? signal_processor::d1( impl_->signal_processor_->at( impl_->signal_processor_->pos_c() ) ) : 0;
    impl_->update_mw();
    impl_->pkfind( impl_->pos_c(), df1, 0 );
}

void
Integrator::close( const adcontrols::PeakMethod& mth, adcontrols::Peaks & peaks, adcontrols::Baselines& baselines )
{
	if ( !impl_->stack_.empty() ) {
		if ( (impl_->stack_[0] == PKTOP) || (impl_->stack_[0] == PKVAL) ) {
			impl_->stack_.push( PEAKSTACK(PKBAS, long(impl_->signal_processor_->d().size() - 1), 0) );
			impl_->pkreduce();
		}
	}

    impl_->assignBaseline();
    // impl_->reduceBaselines();
    // helper::cleanup_baselines( impl_->peaks_, impl_->baselines_ );

    if ( impl_->fixDrift( impl_->peaks_, impl_->baselines_, impl_->drift_ ) ) {
        helper::cleanup_baselines( impl_->peaks_, impl_->baselines_ );
        // impl_->reduceBaselines();
	}

    impl_->updatePeakAreaHeight( mth );
    impl_->rejectPeaks( mth );
    helper::cleanup_baselines( impl_->peaks_, impl_->baselines_ );

#if ! defined NDEBUG && 0
    ADDEBUG() << "integrator::close (final) -- found " << impl_->peaks_.size() << " peaks";
	for ( const auto& pk: impl_->peaks_ ) {
        ADDEBUG() << "\t" << std::make_tuple( pk.startTime(), pk.peakTime(), pk.endTime() );
    }
    ADDEBUG() << "\t, and " << impl_->baselines_.size() << " baselines.";
	for ( const auto& bs: impl_->baselines_ ) {
        ADDEBUG() << "\t" << std::make_tuple(
            std::make_tuple( bs.startTime(), bs.startHeight() ), std::make_tuple( bs.stopTime(), bs.stopHeight() ) );
    }
#endif

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
                impl_->stack_.push( PEAKSTACK(PKBAS, long(impl_->signal_processor_->d().size() - 1), 0) );
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
                helper::updateAreaHeight(*signal_processor_, *pbs, *it);
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
    for ( auto& pk: peaks_ ) {

        pk.setPeakId( id++ );

        helper::tRetention_lsq( *signal_processor_, pk ) || helper::tRetention_moment( *signal_processor_, pk );

        // Peak width
        helper::peak_width( method, *signal_processor_, pk );

        // NTP
        helper::theoreticalplate( method, *signal_processor_, pk );

		// TailingFactor
        helper::asymmetry( method, *signal_processor_, pk );

        // k'
		if ( method.t0() >= 0.001 )
			pk.setCapacityFactor( ( pk.peakTime() - method.t0() ) / method.t0() );
	}
}

void
Integrator::impl::pkfind( long pos, double df1, double )
{
	if ( offIntegration_ )
		return;

    if ( pos <= 2 )
        return;

    int32_t mwup = std::max( int( ( minw_ / signal_processor_->sampInterval() ) / 2 ), 2 );
    auto mwdn = mwup;

    if ( stf_ > 0 ) {
        mwdn = 3;
    } else if ( stf_ < 0 ) {
        mwup = 3;
    }

    auto prev_stf = stf_;

    if ( df1 < -( ss_ ) ) {	/* Down slope */

        if ( dc_++ == 0 ) {
            ld_ = pos; // - mw_ / 2;
            ldd_ = signal_processor_->getIntensity( ld_ );
        }
        uc_ = 0;

    } else if ( df1 > ss_ ) {	/* UP slope */

        if ( uc_++ == 0 ) {
            lu_ = pos;
            lud_ = signal_processor_->getIntensity( ld_ );
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
#if ! defined NDEBUG && 0
    ADDEBUG() << std::make_tuple( pos, signal_processor_->at( pos ) ) << "\t" << std::make_tuple( uc_, dc_, zc_ );
#endif

    if ( stf_ != prev_stf ) {
        if ( prev_stf == PEAK_STATE_NEGATIVE && stf_ == PEAK_STATE_POSITIVE ) {
            assign_valley( pos, ld_ );
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
Integrator::impl::assign_valley( size_t pos, size_t ld )
{
    if ( ld > pos || signal_processor_->d().size() <= pos || signal_processor_->d().size() <= ld )
        return;

    auto it = std::min_element( signal_processor_->d().begin() + ld, signal_processor_->d().begin() + pos
                                , []( const auto& a, const auto& b ){return signal_processor::value(a) < signal_processor::value(b); } );

    auto valley = std::distance(  signal_processor_->d().begin(), it );

    //stack_.push( PEAKSTACK( PKVAL, long( valley ), *it ) );
    stack_.push( PEAKSTACK( PKVAL, long( valley ), signal_processor::value( *it  ) ) );

    pkreduce();
}

void
Integrator::impl::assign_pktop( size_t pos, size_t uppos )
{
	if ( uppos > pos || signal_processor_->d().size() <= pos || signal_processor_->d().size() <= uppos )
		return;

    auto it = std::max_element( signal_processor_->d().begin() + uppos, signal_processor_->d().begin() + pos
                                , [](const auto& a, const auto& b){ return signal_processor::value(a) < signal_processor::value(b); } );
    auto apex = std::distance( signal_processor_->d().begin(), it );

    stack_.push( PEAKSTACK( PKTOP, long( apex ), signal_processor::value( *it ) ) );
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
        while ( --backtrack && ld_ && signal_processor_->getIntensity( ld_ - 1 ) > signal_processor_->getIntensity( ld_ ) )
            --ld_;
        stack_.push( PEAKSTACK( PKSTA, ld_, signal_processor_->getIntensity( ld_ ) ) );
    }  else {
        while ( --backtrack && lu_ && signal_processor_->getIntensity( lu_ - 1 ) < signal_processor_->getIntensity( lu_ ) )
            --lu_;
        stack_.push( PEAKSTACK( PKSTA, lu_, signal_processor_->getIntensity( lu_ ) ) );
    }
}

void
Integrator::impl::pkreduce()
{
    PEAKSTACK sp0 = stack_.top();

#if ! defined NDEBUG && 0
    std::string s;
    for ( int i = stack_.size() - 1; i >= 0; --i )
        s += toChar( stack_[i].stat() );
    ADDEBUG() << "reduce: stack [" << s << "]";
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
                baselines_.emplace_back( helper::baseline( *signal_processor_, sp2.pos(), sp1.pos() ) );
				return;
			} else	{
				return;
			}
		} else {
            peaks_.emplace_back( helper::peak( *signal_processor_, sp2, sp1, sp0 ) );
		}

		stack_.pop();	// remove sp1, PKTOP

		if (sp2 == PKSTA && sp0 == PKBAS)	 { /* SxB */
            baselines_.emplace_back( helper::baseline( *signal_processor_, sp2.pos(), sp0.pos() ) );
			stack_.pop();	// remove sp2, PKSTA

		} else if (sp2 == PKVAL && sp0 == PKBAS) { /* VxB */
			stack_.pop();	// remove sp2, PKVAL
			if (!stack_.empty()) {

				PEAKSTACK sp3 = stack_.top();
				if (sp3 == PKSTA)  { /* SVxB */
                    baselines_.emplace_back( helper::baseline( *signal_processor_, sp3.pos(), sp0.pos() ) );
				}
				stack_.pop();  // remove sp3, PKSTA
			} else {
				ADDEBUG() << "REDUCE : Base did not correspond to Start";
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

namespace {

    struct baseline_levels {
        const adcontrols::Baseline& bs_;
        baseline_levels( const adcontrols::Baseline& bs ) : bs_( bs ) {}
        double drift_height( double t, double drift ) const { return std::max( t - bs_.startTime(), 0.0 ) * drift + bs_.startHeight(); }
    };

    struct VertexProperty {
        std::pair< adcontrols::Peaks::vector_type::iterator
                   , adcontrols::Peaks::vector_type::iterator > ppks;
        void set_properties( std::pair< adcontrols::Peaks::vector_type::iterator
                             , adcontrols::Peaks::vector_type::iterator >&& a ) {
            ppks = std::move( a );
        }
    };

    struct EdgeProperty   {
        adcontrols::Peaks::vector_type::const_iterator ppk;
        void set_properties( adcontrols::Peaks::vector_type::const_iterator a ) {
            ppk = a;
        }
    };

    typedef boost::adjacency_list< boost::vecS
                                   , boost::vecS
                                   , boost::directedS
                                   , VertexProperty
                                   , EdgeProperty > VGraph;
    typedef boost::graph_traits< VGraph >::vertex_descriptor Vertex;
}

bool
Integrator::impl::fixDrift( adcontrols::Peaks& pks, adcontrols::Baselines & bss, double drift )
{
    using adcontrols::Baselines;
    using adcontrols::Baseline;
    using adcontrols::Peaks;

    Baselines fixed( bss );

	for ( Baselines::vector_type::iterator it = bss.begin(); it != bss.end(); ++it ) {

        std::vector< adcontrols::Peaks::vector_type::iterator > shared_peaks;
        for ( auto pk = pks.begin(); pk != pks.end(); ++pk ) {
            if ( pk->baseId() == it->baseId() )
                shared_peaks.emplace_back( pk );
        }
        if ( shared_peaks.size() <= 1 )
            continue;

        VGraph g;
        VGraph::vertex_descriptor v2;
        for ( auto pIt = shared_peaks.begin(); pIt != shared_peaks.end(); ++pIt ) {
            auto v1 = boost::add_vertex( g );
            if ( pIt + 1 != shared_peaks.end() ) {
                v2 = boost::add_vertex( g );
                g[ v1 ].set_properties( { pks.end(), *pIt } );
                g[ v2 ].set_properties( { *pIt,      *(pIt+1) } );
                auto [e, ok] = boost::add_edge(v1, v2, g);
                g[ e ].set_properties( *pIt );
            } else { // last peak := pIt+1 == end
                g[ v1 ].set_properties( { *pIt,       pks.end() } );
                auto [e, ok] = boost::add_edge(v2, v1, g);
                g[ e ].set_properties( *pIt );
            }
        }

        bool fixing( false );
        baseline_levels level( *it );
        BOOST_FOREACH( auto v, boost::vertices( g ) ) {
            auto ppk = g[v].ppks.second;
            if ( ppk != pks.end() ) {
                if ( level.drift_height( ppk->endHeight(), drift ) > ppk->endHeight() || fixing ) {
                    auto& a = fixed.emplace_back( helper::baseline( *signal_processor_, ppk->startPos(), ppk->endPos() ) );
                    ppk->setBaseId( a.baseId() );
                    fixPenetration( a, *ppk );
                    fixing = true;
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
            peaks.emplace_back( ipk );
    }

    if ( ! peaks.empty() ) {   // if not emptry

		if ( bs.startHeight() >= bs.stopHeight() ) {  // negative slope : looking for backword direction

            std::vector<peak_iterator>::reverse_iterator fixup = peaks.rbegin();
            for ( std::vector<peak_iterator>::reverse_iterator ppk = peaks.rbegin(); ppk != peaks.rend() - 1; ++ppk ) {

				if ( intercept(bs, (*ppk)->startPos(), (*ppk)->startHeight()) ) {

					long newId = fixed.add( helper::baseline( *signal_processor_, (*ppk)->startPos(), bs.stopPos()) );

					// modify current baseline
					bs.setStopPos((*ppk)->startPos());
					bs.setStopTime((*ppk)->startTime());
					bs.setStopHeight((*ppk)->startHeight());

                    while ( fixup != ppk )
                        (*fixup++)->setBaseId(newId);
					(*fixup++)->setBaseId( newId ); // := ppk
					res = true;
				}

			}

		} else {
			// if slope is positive then check from front
            std::vector<peak_iterator>::iterator fixup = peaks.begin();

            for ( std::vector<peak_iterator>::iterator ppk = peaks.begin(); ppk != peaks.end() - 1; ++ppk) {

                if ( intercept(bs, (*ppk)->endPos(), (*ppk)->endHeight()) ) {

                    long newId = fixed.add( helper::baseline( *signal_processor_, bs.startPos(), (*ppk)->endPos()) );

					// modify current baseline
                    while ( fixup != ppk )
                        (*fixup++)->setBaseId(newId);
					(*fixup++)->setBaseId( newId ); // := ppk

					bs.setStartPos( (*ppk)->endPos() );
					bs.setStartTime( (*ppk)->endTime() );
                    bs.setStartHeight( (*ppk)->endHeight() );

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

	for ( Peaks::vector_type::iterator it = peaks_.begin(); it != peaks_.end(); ++it ) {
        Peak & pk = *it;
        auto bIt = std::find_if( baselines_.begin(), baselines_.end()
                                 , [&](const auto& b){ return b.startPos() < pk.topPos() && pk.topPos() < b.stopPos(); });
        if ( bIt != baselines_.end() ) {
            pk.setBaseId( bIt->baseId() );
        } else {
            auto& baseline = baselines_.emplace_back( helper::baseline( *signal_processor_, pk.startPos(), pk.endPos()) );
            pk.setBaseId( baseline.baseId() );
            ADDEBUG() << "--- baseline not assigned for peak: " << std::make_tuple( pk.startTime(), pk.peakTime(), pk.endTime() );
        }
	}
}

void
Integrator::impl::fixPenetration( adcontrols::Baseline & bs, const adcontrols::Peak& pk )
{
    std::pair< int, int > offlimits{ bs.startPos(), bs.stopPos() };
    if ( bs.startHeight() < bs.stopHeight() ) { // positive slope -- check front
        std::pair< int, double > t{0, 0.0};
        for ( int pos = pk.startPos(); pos <= pk.topPos(); ++pos ) {
            double d = signal_processor_->getIntensity( pos ) - bs.height( pos );
            if ( d < 0 && d < std::get< 1 >( t ) ) {
                t = { pos, d };
            }
        }
        if ( t.first > 0 )
            offlimits.first = t.first;
    } else { // negative slope -- check tail
        std::pair< int, double > t{0, 0.0};
        for ( int pos = pk.topPos(); pos <= pk.endPos(); ++pos ) {
            double d = signal_processor_->getIntensity( pos ) - bs.height( pos );
            if ( d < 0 && d < std::get< 1 >( t ) ) {
                t = { pos, d };
            }
        }
        if ( t.first > 0 )
            offlimits.second = t.first;
    }

    if ( offlimits.first != bs.startPos() ) {
        bs.setStartPos( offlimits.first );
        bs.setStartTime( signal_processor_->getTime( offlimits.first ) );
        bs.setStartHeight( signal_processor_->getIntensity( offlimits.first ) );
    }
    if ( offlimits.second != bs.stopPos() ) {
        bs.setStopPos( offlimits.second );
        bs.setStopTime( signal_processor_->getTime( offlimits.second ) );
        bs.setStopHeight( signal_processor_->getIntensity( offlimits.second ) );
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
helper::tRetention_lsq(  const signal_processor& c, adcontrols::Peak& pk )
{
	double l_threshold = pk.topHeight() - ( (pk.topHeight() - pk.startHeight()) * 0.5 );
    double r_threshold = pk.topHeight() - ( (pk.topHeight() - pk.endHeight()) * 0.5 );

    // left boundary
    long left_bound = pk.topPos() - 1;
    while ( ( c.getIntensity( left_bound - 1 ) > l_threshold ) && ( ( left_bound - 1 ) > pk.startPos() ) ) {
        left_bound--;
    }

    // right boundary
    long right_bound = pk.topPos() + 1;
    while ( ( c.getIntensity( right_bound + 1 ) > r_threshold ) && ( ( right_bound + 1 ) < pk.endPos() ) ) {
        right_bound++;
    }

	if ( ( right_bound - left_bound + 1 ) < 5 ) {
		left_bound--;
		right_bound++;
	}

    std::vector<double> X, Y;
    for ( long i = left_bound; i <= right_bound; ++i ) {
        X.emplace_back( c.getTime( i ) );
        Y.emplace_back( c.getIntensity( i ) );
    }

    std::vector<double> r;
    if ( adportable::polfit::fit( &X[0], &Y[0], X.size(), 3, r ) ) {
        double a = r[0];
        double b = r[1];
        double c = r[2];
        double tR = (-b) / 2 / c;

        // ADDEBUG() << std::make_tuple( X.front(), X.back() ) << ", tR: " << tR;

        if ( tR < X.front() || X.back() < tR ) {
            // apex is outside range -- no maximum found
            return false;
        }

		pk.setPeakTime( tR );
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

bool
helper::tRetention_moment(  const signal_processor& c, adcontrols::Peak& pk )
{
    adportable::Moment moment( [&](int pos){ return c.getTime( pos ); } );

    double h = pk.topHeight() - std::min( pk.startHeight(), pk.endHeight() );
    double threshold = pk.topHeight() - h * 0.5;

    double cx = moment.centreX( [&](int pos){ return c.getIntensity( pos); }, threshold, pk.startPos(), pk.topPos(), pk.endPos() );
    pk.setPeakTime( cx );

    adcontrols::RetentionTime tr;
    tr.setAlgorithm( adcontrols::RetentionTime::Moment );
    tr.setThreshold( threshold, threshold );
    tr.setBoundary( moment.xLeft(), moment.xRight() );

    pk.setRetentionTime( tr );

    return true;
}

bool
helper::cleanup_baselines( const adcontrols::Peaks& pks, adcontrols::Baselines& bss )
{
	std::set<int> idList;

	for ( adcontrols::Peaks::vector_type::const_iterator it = pks.begin(); it != pks.end(); ++it )
		idList.insert( it->baseId() );

    adcontrols::Baselines::vector_type::iterator pos
        = std::remove_if( bss.begin(), bss.end(), [&](const auto& a){ return idList.find( a.baseId() ) == idList.end(); } );
    if ( pos != bss.end() )
		bss.erase( pos, bss.end() );

	return true;
}

// static
void
helper::updateAreaHeight( const signal_processor& c, const adcontrols::Baseline& bs, adcontrols::Peak& pk )
{
    double area = 0;
    double height = c.getIntensity( pk.topPos() ) - bs.height( pk.topPos() );
    for ( int pos = pk.startPos(); pos <= pk.endPos(); ++pos ) {
        double h = c.getIntensity(pos) - bs.height(pos);
        double w = c.getTime( pos + 1 ) - c.getTime( pos );
        if ( h >= 0.0 ) {
            if ( c.isCounting() )
                area += h;
            else
                area += h * w;
        }
    }
    pk.setPeakArea( area );
    pk.setPeakHeight( height );
}

adcontrols::Peak
helper::peak( const signal_processor& c, const PEAKSTACK& s, const PEAKSTACK& t, const PEAKSTACK& e )
{
    adcontrols::Peak pk;

    pk.setStartPos( s.pos(), c.getIntensity( s.pos() ) );
    pk.setTopPos( t.pos(),   c.getIntensity( t.pos() ) );
    pk.setEndPos( e.pos(),   c.getIntensity( e.pos() ) );

    const uint32_t flags{ uint32_t((s.stat() & 0x0f) << 8) | uint32_t((t.stat() & 0x0f) << 4) | uint32_t(e.stat() & 0x0f) };
    std::string sflags;
    for ( auto f: { s, t, e } )
        sflags += toChar( f.stat() );

    pk.setPeakFlags( flags );
    // ADDEBUG() << "peak flag: " << sflags;

    pk.setStartTime( c.getTime( s.pos() ) );
    pk.setPeakTime( c.getTime( t.pos() ) );
    pk.setEndTime( c.getTime( e.pos() ) );

    return pk;
}

adcontrols::Peak
helper::peak( const signal_processor& c, int spos, int tpos, int epos, unsigned long flags )
{
    adcontrols::Peak pk;

    pk.setStartPos( spos, c.getIntensity( spos ) );
    pk.setTopPos( tpos, c.getIntensity( tpos ) );
    pk.setEndPos( epos, c.getIntensity( epos ) );
    pk.setPeakFlags( flags );

    pk.setStartTime( c.getTime( spos ) );
    pk.setPeakTime( c.getTime( tpos ) );
    pk.setEndTime( c.getTime( epos ) );

    return pk;
}

bool
helper::peak_width( const adcontrols::PeakMethod&, const signal_processor& c, adcontrols::Peak& pk )
{
    adportable::Moment moment( [&](int pos){ return c.getTime( pos ); } );

    double threshold = pk.topHeight() - pk.peakHeight() * 0.5;
    double width = moment.width( [&](int pos){ return c.getIntensity( pos ); }, threshold, pk.startPos(), pk.topPos(), pk.endPos() );
    pk.setPeakWidth( width );

    return true;
}

bool
helper::asymmetry( const adcontrols::PeakMethod&, const signal_processor& c, adcontrols::Peak& pk )
{
    adportable::Moment moment( [&](int pos){ return c.getTime( pos ); } );

    double threshold = pk.topHeight() - pk.peakHeight() * 0.95;
    double width = moment.width( [&](int pos){ return c.getIntensity( pos ); }, threshold, pk.startPos(), pk.topPos(), pk.endPos() );
    double a = pk.peakTime() - moment.xLeft();

    adcontrols::PeakAsymmetry tf;

    tf.setAsymmetry( width / (2 * a) );
    tf.setBoundary( moment.xLeft(), moment.xRight() );

    pk.setAsymmetry( tf );

    return true;
}

bool
helper::theoreticalplate( const adcontrols::PeakMethod&, const signal_processor& c, adcontrols::Peak& pk )
{
    adportable::Moment moment( [&](int pos){ return c.getTime( pos ); } );

    double threshold = pk.topHeight() - pk.peakHeight() * 0.5;
    double width = moment.width( [&](int pos){ return c.getIntensity( pos ); }, threshold, pk.startPos(), pk.topPos(), pk.endPos() );
    double N = 5.54 * ( ( pk.peakTime() / width ) * ( pk.peakTime() / width ) );

    adcontrols::TheoreticalPlate ntp;
    ntp.ntp( N );

    pk.setTheoreticalPlate( ntp );

    return true;
}

///////////////

adcontrols::Baseline
helper::baseline( const signal_processor& c, int spos, int epos )
{
    adcontrols::Baseline bs;

    bs.setStartPos( spos );
    bs.setStopPos( epos );
    bs.setStartTime( c.getTime( spos ) );
    bs.setStopTime( c.getTime( epos ) );
    bs.setStartHeight( c.getIntensity( spos ) );
    bs.setStopHeight( c.getIntensity( epos ) ) ;

    return bs;
}

/////////////////////////////
void
Integrator::impl::update_params()
{
    int mw = mw_;
    if ( sampInterval_ && *sampInterval_ > 0.01 ) {
        mw = std::max( int( minw_ / *sampInterval_ ) | 1, 3 ); // should be odd number
    } else if ( signal_processor_->sampInterval() > 0.01 && minw_ > 0.01 ) { // grater than 10 ms
        mw = std::max( int( minw_ / signal_processor_->sampInterval() ) | 1, 3 ); // should be odd number
    }
    mw = std::min( mw, 31 ); // no larger than 31
    if ( mw_ != mw ) {
        mw_ = mw;
        signal_processor_->set_ndiff( mw_ );
    }
}

void
Integrator::impl::update_mw()
{
    if ( signal_processor_->d().size() >= 2 ) {
        int mw = std::max( 3, int( minw_ / signal_processor_->sampInterval() ) | 1 );
        if ( mw != mw_ ) {
            // ADDEBUG() << "\t------------ " << __FUNCTION__ << " -----> " << std::make_tuple( mw_, " --> ", mw, minw_, signal_processor_->sampInterval() );
            mw_ = mw; // should grator or equal to 3
            signal_processor_->set_ndiff( mw_ );
        } else {
            // if ( signal_processor_->d().size() == 2 )
            //     ADDEBUG() << "\t------------ " << __FUNCTION__ << " -----> " << std::make_tuple( mw_, " --> ", mw, minw_, signal_processor_->sampInterval() );
        }
    }
}
