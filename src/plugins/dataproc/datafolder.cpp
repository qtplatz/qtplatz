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
#include <boost/variant/static_visitor.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <variant>

namespace {
    // helper type for the visitor
    template<class... Ts>
    struct overloads : Ts... { using Ts::operator()...; };
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

    if ( auto var = adutils::to_std_variant< dataTuple >()( static_cast< const boost::any& >( folium ) ) ) {
        const auto f_visitor = overloads{
            [&]( std::shared_ptr< adcontrols::Chromatogram > ptr ) {
                if ( not ptr->display_name() )
                    ptr->set_display_name( folium.name<char>() );
                this->chromatogram_ = ptr;
                this->isCounting_ = ( ptr && ptr->isCounting() );
            },
            [&]( std::shared_ptr< adcontrols::MassSpectrum > ptr ) {
                this->primary_ = ptr;
                this->isCounting_ = ( ptr && ptr->isHistogram() );
            },
            []<typename T>( T ) {
            }
        };
        std::visit( f_visitor, *var );
        for ( auto& a: folium.attachments() ) {
            if ( auto var = adutils::to_std_variant< dataTuple >()(static_cast< const boost::any& >( a )) ) {
                // boost::apply_visitor( attachment_visitor( this, a ), *var );
                const auto a_visitor = overloads{
                    [&]< typename T > ( T ) {
                        ADDEBUG() << "unhandled attachment: " << a.name() << "\t" << typeid(T).name();
                    },
                    [&]( std::shared_ptr< adcontrols::PeakResult > ptr ) {
                        this->peakResult_ = ptr;
                    },
                    [&]( std::shared_ptr< adcontrols::MassSpectrum > ptr ) {
                        if ( a.name() == Constants::F_PROFILED_HISTOGRAM ) {
                            this->profiledHistogram_ = ptr;
                            if ( ptr ) {
                                this->overlaySpectrum_ = std::make_shared< adcontrols::MassSpectrum >( *ptr );
                                auto n = ptr->getMSProperty().samplingInfo().numberOfTriggers();
                                for ( size_t i = 0; i < ptr->size(); ++i ) {
                                    this->overlaySpectrum_->setIntensity( i, ptr->intensity( i ) * 1000.0 / n );
                                }
                            }
                        } else if ( a.name() == Constants::F_CENTROID_SPECTRUM ) {
                            this->centroid_ = ptr;
                        } else {
                            ADDEBUG() << "\t-- attachment <MassSpectrum> not handled: " << folium_.name();
                        }
                    },
                    [&]( std::shared_ptr< adcontrols::Chromatogram > ptr ) {
                        // ADDEBUG() << "\t-- attachment <Chromatogram> not handled: " << folium_.name();
                    }
                };
                std::visit( a_visitor, *var );
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
datafolder::make_display_name( const std::filesystem::path& path, const portfolio::Folium& folium )
{
    const char inserter = ';';

    auto rpath = std::filesystem::relative( path, path / "../.." );
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
