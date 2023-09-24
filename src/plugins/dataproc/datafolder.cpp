/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
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

#include "datafolder.hpp"
#include "constants.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/histogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adutils/processeddata_t.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace dataproc {

    struct datafolder::attachment_visitor : public boost::static_visitor< void > {
        datafolder* this_;
        const portfolio::Folium& folium_;
        attachment_visitor( datafolder* t, const portfolio::Folium& f ) : this_( t ), folium_( f ) {}

        template< typename T > void operator () ( T ) const {
            ADDEBUG() << "unhandled attachment: " << folium_.name() << "\t" << typeid(T).name();
        };

        void operator () ( std::shared_ptr< adcontrols::PeakResult > ptr ) const {
            this_->peakResult_ = ptr;
        }

        void operator () ( std::shared_ptr< adcontrols::MassSpectrum > ptr ) const {
            if ( folium_.name() == Constants::F_PROFILED_HISTOGRAM ) {
                this_->profiledHistogram_ = ptr;
                if ( ptr ) {
                    this_->overlaySpectrum_ = std::make_shared< adcontrols::MassSpectrum >( *ptr );
                    auto n = ptr->getMSProperty().samplingInfo().numberOfTriggers();
                    for ( size_t i = 0; i < ptr->size(); ++i ) {
                        this_->overlaySpectrum_->setIntensity( i, ptr->intensity( i ) * 1000.0 / n );
                    }
                }
            } else if ( folium_.name() == Constants::F_CENTROID_SPECTRUM ) {
                this_->centroid_ = ptr;
            } else {
                ADDEBUG() << "\t-- attachment <MassSpectrum> not handled: " << folium_.name();
            }
        }

        void operator () ( std::shared_ptr< adcontrols::Chromatogram > ptr ) const {
            // ADDEBUG() << "\t-- attachment <Chromatogram> not handled: " << folium_.name();
        }
    };

    struct datafolder::folium_visitor : public boost::static_visitor< void > {
        datafolder* this_;
        const portfolio::Folium& folium_;
        folium_visitor( datafolder* t, const portfolio::Folium& f ) : this_( t ), folium_( f ) {};

        template< typename T > void operator () ( T ) const {
            ADDEBUG() << "unhandled folium: " << folium_.name() << "\t" << typeid(T).name();
        };
        void operator () ( std::shared_ptr< adcontrols::Chromatogram > ptr ) const {
            this_->chromatogram_ = ptr;
            this_->isCounting_ = ( ptr && ptr->isCounting() );
        }
        void operator () ( std::shared_ptr< adcontrols::MassSpectrum > ptr ) const {
            this_->primary_ = ptr;
            this_->isCounting_ = ( ptr && ptr->isHistogram() );
        }
    };
}

using namespace dataproc;


datafolder::datafolder() : idx_(0)
                         , isCounting_( false )
                         , isChecked_( false )
{
}

datafolder::datafolder( const Dataprocessor * dp
                        , const portfolio::Folium& folium ) : idx_( 0 )
                                                            , display_name_( make_display_name( dp->filename(), folium ) )
                                                            , folium_( folium )
                                                            , isCounting_( false )
                                                            , isChecked_( folium.attribute( "isChecked" ) == "true" )
{
    using dataTuple = std::tuple< std::shared_ptr< adcontrols::PeakResult >
                                  , std::shared_ptr< adcontrols::Chromatogram >
                                  , std::shared_ptr< adcontrols::MassSpectrum >
                                  >;

    if ( auto var = adutils::to_variant< dataTuple >()( static_cast< const boost::any& >( folium ) ) ) {
        boost::apply_visitor( folium_visitor(this, folium), *var );

        for ( auto& a: folium.attachments() ) {
            if ( auto var = adutils::to_variant< dataTuple >()(static_cast< const boost::any& >( a )) ) {
                boost::apply_visitor( attachment_visitor( this, a ), *var );
            }
        }
    }

    if ( auto profile = primary_.lock() ) {
        if ( profile->isHistogram() && !profiledHistogram_.lock() ) {
            self_profiled_histogram_ = adcontrols::histogram::make_profile( *profile, *dp->massSpectrometer() );
            profiledHistogram_ = self_profiled_histogram_;
        }
    }

}

datafolder::datafolder( const datafolder& t ) : idx_( t.idx_ )
                                              , display_name_( t.display_name_ )
                                              , folium_( t.folium_ )
                                              , primary_( t.primary_ )
                                              , profiledHistogram_( t.profiledHistogram_ )
                                              , centroid_( t.centroid_ )
                                              , chromatogram_( t.chromatogram_ )
                                              , peakResult_( t.peakResult_ )
                                              , overlaySpectrum_( t.overlaySpectrum_ )
                                              , overlayChromatogram_( t.overlayChromatogram_ )
                                              , isCounting_( t.isCounting_ )
                                              , isChecked_( t.isChecked_ )
{
}

//static
QString
datafolder::make_display_name( Dataprocessor * dp, const portfolio::Folium& folium )
{
    auto pfolio = dp->getPortfolio();
    return make_display_name( pfolio.fullpath(), folium );
}

QString
datafolder::make_display_name( const std::wstring& fullpath, const portfolio::Folium& folium )
{
    const char inserter = ';';

    boost::filesystem::path path( fullpath );
    auto rpath = boost::filesystem::relative( path, path / "../.." );
    std::wstring name = rpath.wstring() + wchar_t( inserter ) + boost::algorithm::trim_copy( folium.name() );
    return QString::fromStdWString( name );
}

datafolder::operator bool() const
{
    return ( primary_.lock() || chromatogram_.lock() );
}

int
datafolder::idx() const
{
    return idx_;
}

void
datafolder::setIdx( int idx )
{
    idx_ = idx;
}

template<>
std::string
datafolder::filename() const
{
    return folium_.filename<char>();
}

template<>
std::wstring
datafolder::filename() const
{
    return folium_.filename<wchar_t>();
}

QString datafolder::display_name() const
{
    return display_name_;
}

portfolio::Folium&
datafolder::folium()
{
    return folium_;
}

const portfolio::Folium&
datafolder::folium() const
{
    return folium_;
}

boost::uuids::uuid
datafolder::uuid() const
{
    return folium_.uuid();
}


std::shared_ptr< const adcontrols::MassSpectrum >
datafolder::get_profiled_histogram() const
{
    if ( auto ppkd = profiledHistogram_.lock() )
        return ppkd;
    return {};
}

std::shared_ptr< const adcontrols::MassSpectrum >
datafolder::get_primary_spectrum() const
{
    return primary_.lock();
}


boost::optional< std::pair< std::shared_ptr< const adcontrols::MassSpectrum >, bool /* isHistogram */> >
datafolder::get_primary() const
{
    if ( auto prof = this->primary_.lock() )
        return {{ prof, prof->isHistogram() }};
    else
        return {};
}

boost::optional< std::pair< std::shared_ptr< const adcontrols::MassSpectrum >, bool /* isHistogram */> >
datafolder::get_processed() const
{
    if ( auto ms = this->centroid_.lock() ) {
        return {{ ms, ms->isHistogram() }};
    } else {
        if ( auto profile = this->primary_.lock() ) {
            if ( profile->isCentroid() && !profile->isHistogram() )
                return {{ profile, false }};
        }
        return {};
    }
}

boost::optional< std::pair< std::shared_ptr< const adcontrols::MassSpectrum >, bool > >
datafolder::get_spectrum_for_overlay() const
{
    if ( auto ppkd = profiledHistogram_.lock() ) {
        if ( overlaySpectrum_ )
            return {{ overlaySpectrum_, true }};
        return {{ ppkd, true }};
    }

    if ( auto prime = primary_.lock() ) {
        if ( ! prime->isCentroid() )
            return {{ prime, false }};
    }
    return {};
}

std::shared_ptr< adcontrols::Chromatogram >
datafolder::get_chromatogram() const
{
    return chromatogram_.lock();
}

std::shared_ptr< adcontrols::PeakResult >
datafolder::get_peakResult() const
{
    return peakResult_;
}
