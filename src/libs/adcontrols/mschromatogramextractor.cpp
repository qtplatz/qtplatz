/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "mschromatogramextractor.hpp"
#include "chromatogram.hpp"
#include "description.hpp"
#include "descriptions.hpp"
#include "lcmsdataset.hpp"
#include "lockmass.hpp"
#include "massspectrum.hpp"
#include "mschromatogrammethod.hpp"
#include "msproperty.hpp"
#include "processmethod.hpp"
#include <adportable/spectrum_processor.hpp>
#include <boost/format.hpp>
#include <numeric>

namespace adcontrols {

    namespace mschromatogramextractor {

        template< typename It > struct accumulate {
            size_t size_;
            const It xbeg_;
            const It xend_;        
            const It y_;

            accumulate( It x, It y, size_t size ) : size_( size )
                                                  , xbeg_( x )
                                                  , xend_( x + size )
                                                  , y_( y ) {
            }
        
            double operator()( double lMass, double uMass ) const {
                if ( size_ ) {
                    auto lit = std::lower_bound( xbeg_, xend_, lMass );
                    if ( lit != xend_ ) {
                        auto bpos = std::distance( xbeg_, lit );
                        auto uit = std::lower_bound( xbeg_, xend_, uMass );
                        if ( uit == xend_ )
                            uit--;
                        while ( uMass < *uit )
                            --uit;
                        auto epos = std::distance( xbeg_, uit );
                        if ( bpos > epos )
                            epos = bpos;

                        return std::accumulate( y_ + bpos, y_ + epos, 0.0 );
                    }
                }
                return 0.0;
            }
        };

        /////////////////////////////////////
        class xChromatogram {
            xChromatogram( const xChromatogram& ) = delete;
            xChromatogram& operator = ( const xChromatogram& ) = delete;
        public:
            xChromatogram( const MSChromatogramMethod::value_type& target
                           , double width
                           , uint32_t fcn
                           , uint32_t target_index ) : fcn_( fcn )
                                                     , target_index_( target_index )
                                                     , target_( target )
                                                     , count_( 0 )
                                                     , pchr_( std::make_shared< adcontrols::Chromatogram >() )  {
                pchr_->addDescription(
                    adcontrols::description( L"Create"
                                             , ( boost::wformat( L"%s %.4f (%.3gmDa)" )
                                                 % adportable::utf::to_wstring( target.formula )
                                                 % target.mass
                                                 % ( width * 1000 )
                                                 ).str() ) );
                
            }

            void append( uint32_t pos, double time, double y ) {

                if ( count_++ == 0 && pos > 0 )
                    return; // ignore first data after chromatogram condition change

                (*pchr_) << std::make_pair( time, y );
                pos_ = pos;

            }

            uint32_t fcn_;
            uint32_t target_index_;
            uint32_t pos_; // last pos
            uint32_t count_; // data count
            MSChromatogramMethod::value_type target_;
            std::shared_ptr< adcontrols::Chromatogram > pchr_;
            
        };
    }
    
    class MSChromatogramExtractor::impl {
    public:
        impl( const adcontrols::LCMSDataset * raw ) : raw_( raw )
            {}
        
        void prepare_spectra( size_t pos, std::shared_ptr< adcontrols::MassSpectrum > );
        void create_chromatograms( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                   , const adcontrols::MSChromatogramMethod& m );
        void append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, const adcontrols::MSChromatogramMethod& );
        size_t read_raw_spectrum( size_t pos, const adcontrols::LCMSDataset * raw, adcontrols::MassSpectrum& );

        std::vector< std::shared_ptr< mschromatogramextractor::xChromatogram > > results_;

        std::map< size_t, std::shared_ptr< adcontrols::MassSpectrum > > spectra_;
        const adcontrols::LCMSDataset * raw_;
    };
}

using namespace adcontrols;

MSChromatogramExtractor::~MSChromatogramExtractor()
{
    delete impl_;
}

MSChromatogramExtractor::MSChromatogramExtractor( const adcontrols::LCMSDataset * raw ) : impl_( new impl( raw ) )
{
}

bool
MSChromatogramExtractor::operator () ( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                       , const adcontrols::ProcessMethod& pm
                                       , std::function<bool( size_t, size_t )> progress )
{
    if ( auto cm = pm.find< adcontrols::MSChromatogramMethod >() ) {
    
        adcontrols::Chromatogram tic;
        size_t nSpectra(0);
        if ( impl_->raw_ && impl_->raw_->getTIC( 0, tic ) )
            nSpectra = tic.size();

        if ( nSpectra == 0 )
            return false;

        progress( 0, nSpectra );

        impl_->results_.clear();

        size_t pos = 0;
        size_t n( 0 );
        do {
            auto ms = std::make_shared< adcontrols::MassSpectrum >();
            if ( ( pos = impl_->read_raw_spectrum( pos, impl_->raw_, *ms ) ) ) {
                impl_->prepare_spectra( pos - 1, ms );
                impl_->append_to_chromatogram( pos - 1, *ms, *cm );
                if ( progress( ++n, nSpectra ) )
                    return false;
            }
        } while ( pos );

        std::pair< double, double > time_range =
            std::make_pair( impl_->spectra_.begin()->second->getMSProperty().timeSinceInjection()
                          , impl_->spectra_.rbegin()->second->getMSProperty().timeSinceInjection() );
        
        for ( auto& r : impl_->results_ ) {
            r->pchr_->minimumTime( time_range.first );
            r->pchr_->maximumTime( time_range.second );
            vec.push_back( r->pchr_ );
        }

        return true;
    }
    return false;
}

size_t
MSChromatogramExtractor::impl::read_raw_spectrum( size_t pos, const adcontrols::LCMSDataset * raw, adcontrols::MassSpectrum& ms )
{
    int idx, fcn, rep;
    while ( raw->index( pos, idx, fcn, rep ) && fcn != 0 )  // skip until 'fcn = 0' data 
        ++pos;
    if ( raw->getSpectrum( -1, pos, ms ) ) // read all corresponding segments
        return pos + 1;
    return 0;
}


void
MSChromatogramExtractor::impl::prepare_spectra( size_t pos, std::shared_ptr< adcontrols::MassSpectrum > ms )
{
    bool locked = false;

    adcontrols::lockmass lkms;
    if ( raw_->mslocker( lkms ) ) // if on-the-fly acquisition lock-mass applied
        locked = lkms( *ms, true );

    spectra_[ pos ] = ms; // keep processed profile spectrum for second phase
}


void
MSChromatogramExtractor::impl::append_to_chromatogram( size_t pos, const adcontrols::MassSpectrum& ms, const adcontrols::MSChromatogramMethod& cm )
{
    using namespace adcontrols::mschromatogramextractor;
    
    adcontrols::segment_wrapper<const adcontrols::MassSpectrum> segments( ms );

    double width = cm.width( cm.widthMethod() );

    uint32_t fcn = 0;

    for ( auto& fms: segments ) {
        
        double time = fms.getMSProperty().timeSinceInjection();

        uint32_t target_index = 0; //  index to the cm.targets();

        for ( auto& m : cm.targets() ) {

            if ( ! m.enable )
                continue;

            double width = cm.width_at_mass( m.mass ); // cm.width( cm.widthMethod() );
            double lMass = m.mass - width / 2;
            double uMass = m.mass + width / 2;

            if ( fms.getMass( 0 ) <= lMass && uMass < fms.getMass( fms.size() - 1 ) ) {
                double y( 0 );
                if ( fms.isCentroid() ) {
                    y = accumulate<const double *>( fms.getMassArray(), fms.getIntensityArray(), fms.size() )( lMass, uMass );
                } else {
                    double base, rms;
                    double tic = adportable::spectrum_processor::tic( fms.size(), fms.getIntensityArray(), base, rms );
                    adportable::spectrum_processor::areaFraction fraction;
                    adportable::spectrum_processor::getFraction( fraction, fms.getMassArray(), fms.size(), lMass, uMass );

                    y = adportable::spectrum_processor::area( fraction, base, fms.getIntensityArray(), fms.size() );
                }
                
                auto chro = std::find_if( results_.begin(), results_.end()
                                          , [=] ( std::shared_ptr<xChromatogram>& c ) { return c->fcn_ == fcn && c->target_index_ == target_index; } );
                if ( chro == results_.end() ) {
                    results_.push_back( std::make_shared< xChromatogram >( m, width, fcn, target_index ) );
                    chro = results_.end() - 1;
                }
                ( *chro )->append( uint32_t( pos ), time, y );
            }
                        
            target_index++;
        }
        ++fcn;    
    }
}

