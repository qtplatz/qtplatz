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
#include "lcmsdataset.hpp"
#include "massspectrum.hpp"

namespace adcontrols {
    
    class MSChromatogramExtractor::impl {
    public:
        impl( const adcontrols::LCMSDataset * raw ) : raw_( raw )
            {}
        
        void prepare_spectra( size_t pos, std::shared_ptr< adcontrols::MassSpectrum > );
        void create_chromatograms( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                   , const adcontrols::MSChromatogramMethod& m );
        void append_to_chromatogram( size_t pos, std::shared_ptr<const adcontrols::MassSpectrum> ms );
        size_t read_raw_spectrum( size_t pos, const adcontrols::LCMSDataset * raw, adcontrols::MassSpectrum& );

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
MSChromatogramExtractor::operator () ( std::vector< std::shared_ptr< adcontrols::Chromatogram > > vec
                                       , const MSChromatogramMethod& cm
                                       , std::function<bool( size_t, size_t )> progress )
{
    adcontrols::Chromatogram tic;
    size_t nSpectra(0);
    if ( impl_->raw_ && impl_->raw_->getTIC( 0, tic ) )
        nSpectra = tic.size();

    if ( nSpectra == 0 )
        return false;

    progress( 0, nSpectra );
    
    size_t pos = 0;
    size_t n( 0 );
    do {
        auto ms = std::make_shared< adcontrols::MassSpectrum >();
        if ( ( pos = impl_->read_raw_spectrum( pos, impl_->raw_, *ms ) ) ) {
            impl_->prepare_spectra( pos - 1, ms );
            if ( progress( ++n, nSpectra ) )
                return false;
        }
    } while ( pos );
    
    return true;
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
    // correct_baseline( *ms );
#if 0
    if ( mslockm_ && mslock_ ) {
        bool locked = false;
        if ( auto raw = sampleprocessor.getLCMSDataset() ) {
            adcontrols::lockmass lkms;
            if ( raw->mslocker( lkms ) )
                locked = lkms( *ms, true );
        }
        if ( !locked )
            doMSLock( *ms );
    }
#endif
    spectra_[ pos ] = ms; // keep processed profile spectrum for second phase
}

#if 0
void
MSChromatogramExtractor::create_chromatograms( std::vector< std::shared_ptr< adcontrols::Chromatogram > >& vec
                                             , const adcontrols::MSChromatogramMethod& m )
{
    for ( auto& sp : spectra_ ) {
        
    }
}

void
MSChromatogramExtractor::append_to_chromatogram( size_t pos, std::shared_ptr<const adcontrols::MassSpectrum> ms )
{
    adcontrols::segment_wrapper<const adcontrols::MassSpectrum> segments( *ms );

    uint32_t fcn = 0;

    for ( auto& fms: segments ) {
        
        double time = fms.getMSProperty().timeSinceInjection();

        uint32_t candidate_index = 0; //  index to masses_

        for ( auto& m : target_values_ ) {

            double lMass = m.matchedMass - m.width / 2;
            double uMass = m.matchedMass + m.width / 2;

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

                auto chro = std::find_if( begin(), end()
                                          , [=] ( std::shared_ptr<QuanChromatogram>& c ) { return c->fcn() == fcn && c->candidate_index() == candidate_index; } );
                if ( chro == end() ) {
                    qchro_.push_back( std::make_shared< QuanChromatogram >( fcn, candidate_index, formula_, m.exactMass, m.matchedMass, std::make_pair( lMass, uMass ) ) );
                    chro = qchro_.end() - 1;
                }
                ( *chro )->append( uint32_t( pos ), time, y );
            }
                        
            candidate_index++;
        }
        ++fcn;    
    }
}


#endif
