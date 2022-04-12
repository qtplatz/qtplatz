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
#include <adcontrols/massspectrum.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adutils/processeddata_t.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/variant/static_visitor.hpp>

using namespace dataproc;

namespace {

    struct attachment_visitor : public boost::static_visitor< void > {
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
                // ADDEBUG() << "\t-- attachment Profiled histogram: " << folium_.name();
            } else if ( folium_.name() == Constants::F_CENTROID_SPECTRUM ) {
                this_->centroid_ = ptr;
            } else {
                ADDEBUG() << "\t-- attachmenr <MassSpectrum> not handled: " << folium_.name();
            }
        }
    };

    struct folium_visitor : public boost::static_visitor< void > {
        datafolder* this_;
        const portfolio::Folium& folium_;
        folium_visitor( datafolder* t, const portfolio::Folium& f ) : this_( t ), folium_( f ) {};

        template< typename T > void operator () ( T ) const {
            ADDEBUG() << "unhandled folium: " << folium_.name() << "\t" << typeid(T).name();
        };
        void operator () ( std::shared_ptr< adcontrols::Chromatogram > ptr ) const {
            this_->chromatogram_ = ptr;
        }
        void operator () ( std::shared_ptr< adcontrols::MassSpectrum > ptr ) const {
            this_->profile_ = ptr;
        }
    };
}

datafolder::datafolder() : idx_(0)
{
}

datafolder::datafolder( const std::wstring& fullpath
                        , const portfolio::Folium& folium ) : idx_( 0 )
                                                            , filename_( fullpath )
                                                            , display_name_( make_display_name( fullpath, folium ) )
                                                            , idFolium_( folium.id() )
                                                            , idfolium_( folium.uuid() )
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
    ADDEBUG() << "--- datafolder ctor"
              << " has chromatogram ? " << bool(chromatogram_.lock())
              << ", has peakResult ? "  << bool(peakResult_)
              << ", has profile ? "     << bool(profile_.lock())
              << ", has histogram ? "   << bool(profiledHistogram_.lock())
              << ", has centroid ? "    << bool(centroid_.lock());
#if 0
    if ( auto raw = folium.get< adcontrols::MassSpectrumPtr >() ) {
        profile_ = *raw; // maybe profile or histogram
        if ( (*raw)->isHistogram() ) {
            if ( auto fi = portfolio::find_first_of( folium.attachments()
                                                     , [](const auto& a){ return a.name() == Constants::F_PROFILED_HISTOGRAM; }) ) {
                if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( fi ) ) { // no type check any_cast
                    profiledHistogram_ = ptr;
                }
            }
        }
        if ( auto fi = portfolio::find_last_of( folium.attachments()
                                                , [](const auto& a){ return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
            if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( fi ) ) {
                centroid_ = ptr;
            }
        }
    } else if ( auto raw = folium.get< adcontrols::ChromatogramPtr >() ) {
        chromatogram_ = (*raw);
    }
#endif
}

datafolder::datafolder( const datafolder& t ) : idx_( t.idx_ )
                                              , filename_( t.filename_ )
                                              , display_name_( t.display_name_ )
                                              , idFolium_( t.idFolium_ )
                                              , idCentroid_( t.idCentroid_ )
                                              , profile_( t.profile_ )
                                              , profiledHistogram_( t.profiledHistogram_ )
                                              , centroid_( t.centroid_ )
                                              , chromatogram_( t.chromatogram_ )
                                              , overlaySpectrum_( t.overlaySpectrum_ )
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
    return ( profile_.lock() || chromatogram_.lock() );
}

boost::optional< std::pair< std::shared_ptr< const adcontrols::MassSpectrum >, bool /* isHistogram */> >
datafolder::get_profile() const
{
    if ( auto hist = this->profiledHistogram_.lock() )
        return {{ hist, true }};
    else if ( auto prof = this->profile_.lock() )
        return {{ prof, false }};
    else
        return {};
}

boost::optional< std::pair< std::shared_ptr< const adcontrols::MassSpectrum >, bool /* isHistogram */> >
datafolder::get_processed() const
{
    if ( auto ms = this->centroid_.lock() ) {
        if ( auto hist = this->profiledHistogram_.lock() )
            return {{ ms, true }};
        else
            return {{ ms, false }};
    } else {
        return {};
    }
}

std::shared_ptr< adcontrols::Chromatogram >
datafolder::get_chromatogram() const
{
    return chromatogram_.lock();
}
